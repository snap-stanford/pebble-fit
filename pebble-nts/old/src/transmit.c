#include "transmit.h"

// HEAP MEMORY USAGE : 932B

// The purpose of this file is to separate sending to phone
// and sending to server.

// NOTE, final decision. WE send ALL data, each time ANY
// data is sent. This essentially lets us get a snap shot

// MOREOVER (I forgort this), we MUST timestamp the beginning
// of each array so that it can be uniquely added to the local
// storage of the phone, AND so it can be uniquely added to the
// Amazon key. HOWEVER, remember that if we add another timestamp
// to the Amazon key for point at which the element was actually
// uploaded to the server, then we won't have unique points. We
// SHOULD actually add a timestamp for when the data was completed
// and uploaded to the key on the SERVER side, cause the Pebble's
// timestamp might be off if it is not synced properly.

// Anyway, remember, if we try to upload to the server 3 times a day
// then when we push to the phone we will overwrite whatever data that
// shares its type and leading timestamp. So, at most, 3 times a day
// is when we will upload unique data.

// ANYWAY, final decision is that WHENEVER we transmit to the phone,
// we push ALL data, which includes
// Continuously tracked data
//   1. raw actigraphy data
//   2. daily summary data from the the last week (28 days?)
// Patient entered data
// Configuration data
//   1. patient metrics
//   2. wakeup configurataion
//
// we need a way to retry

static int16_t retry_count = 0;
static DictionaryIterator *outbox_iter;

static Window *s_transmit_phone_window;
static Layer *window_layer;

static TextLayer *s_transmit_text_layer;
static TextLayer *s_transmit_countdown_text_layer;

static int16_t trans_code_lcl;
static AppTimer *app_timer_push_to_server;

static uint16_t cur_countdown;
static int16_t num_retry = 10;
static bool countdown_active = false;

static void countdown_timer_handler(void *data);


/* ++++++++++++++++++++++++++++++++++ */
/*     HELPER FUNCTIONS   */
/* ++++++++++++++++++++++++++++++++++ */


static void log_codes_transmit(int16_t err_code){
  APP_LOG(APP_LOG_LEVEL_ERROR, "at: log_codes_transmit(%d)",(int) err_code);
}

static void close_transmit_window(){
  window_stack_remove(s_transmit_phone_window,false);
}


static void decrement_legal_I_BLK_PERSIST_KEY(){
  uint16_t i_blk = persist_read_int(I_BLK_PERSIST_KEY);
  persist_write_int(I_BLK_PERSIST_KEY,((i_blk > 0) ? (i_blk-1) : 0 ));
}


// this function simply calcuates if we should even try to send data
// example: there is no new config,pinteract,actigraphy data, but somehow
// the send to
static bool data_to_send(){
  uint8_t blk_buf[MAX_PINTERACT_PS_B_SIZE];
  persist_read_data(PIRPS_B1_PERSIST_KEY, blk_buf, sizeof(blk_buf));
  uint16_t n_B_pinteract_ps = read_res_buf_byte_count(blk_buf, PINTERACT_PS_B_COUNT_IND);

  // See if ANY of the conditions are true
  if((n_B_pinteract_ps > PINTERACT_PS_HEAD_B_SIZE)
      || (persist_read_int(I_BLK_PERSIST_KEY) > 0)
  ){
    return true;
  }else{
    return false;
  }
}


/* ++++++++++++++++++++++++++++++++++ */
/* ATTEMPT TO TRANSMIT PUSH TO SERVER */
/* ++++++++++++++++++++++++++++++++++ */


static void attempt_to_transmit_push_to_server(){
  log_codes_transmit(15);
  AppMessageResult am_result=  app_message_outbox_begin(&outbox_iter);

  if( am_result == APP_MSG_OK ) {
    int16_t msg_to_server = 0;
    msg_to_server = 1;
    dict_write_int(outbox_iter, AMKEY_PUSHTOSERVER, &msg_to_server, 4,true);
    dict_write_end(outbox_iter);
    app_message_outbox_send();
    // log_codes_transmit(1);
    // APP_LOG(APP_LOG_LEVEL_ERROR, "sent the push to server message of size %d",(int) dict_size(outbox_iter));
  }
}

static void outbox_failed_push_to_server_cb(DictionaryIterator *iterator, AppMessageResult reason, void *context){
  //do nothing and wait for the next chance to transmit
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Transmit dropped!");
  log_codes_transmit(2);

  //   window_stack_pop(false);
  if(bluetooth_connection_service_peek() && (retry_count < num_retry)){
    retry_count += 1; // increase the retry count with each retry
    // psleep(10); // perhaps a second to reset?
    attempt_to_transmit_push_to_server();
  }else{
    // reset the retry count once we have finished re-trying to transmit
    retry_count = 0;
    // we can exit the transmission in only two ways. 1. we acknowledge that the
    // message has been recieved, or 2. that the timer runs out.
  }
}

static void outbox_sent_push_to_server_cb(DictionaryIterator *iterator, void *context){
  log_codes_transmit(3);
  // APP_LOG(APP_LOG_LEVEL_ERROR, "push to server Transmit succeeded!");
  // we reset the retry counter, because we succeeded in sending a message
  retry_count = 0;
  // now that have signaled phone to push to the server, we wait for the
  // countdown to finish or to get an appmessage that all signals were sent
}

static void setup_attempt_to_transmit_push_to_server(){
  // NOTE, the outbox must be zero or else we run out of heap space
  app_message_register_outbox_failed(outbox_failed_push_to_server_cb);
  app_message_register_outbox_sent(outbox_sent_push_to_server_cb);
  APP_LOG(APP_LOG_LEVEL_ERROR, "transmit rem heap B: %d",(int) heap_bytes_free());
}

/* ++++++++++++++++++++++++++++++++++ */
/* ATTEMPT TO TRANSMIT DATA TO PHONE */
/* ++++++++++++++++++++++++++++++++++ */

static void attempt_transmit_data_to_phone(){

  // IF we get this far, we know we have SOMEthing to send over
  // to the phone, even if it is just the flag to transmit to server
  log_codes_transmit(4);
  // APP_LOG(APP_LOG_LEVEL_ERROR, "got to attempt_transmit_data_to_phone");
  AppMessageResult am_result=  app_message_outbox_begin(&outbox_iter);

  if( am_result == APP_MSG_OK ) {

    // declare VARIABLES
    uint8_t blk_buf[PERSIST_DATA_MAX_LENGTH] = {0};
    uint32_t size_ps_blk_acti = 0;
    uint32_t size_ps_blk_pi = 0;

    // transmit pinteract and actigraphy data to phone
    // get acti data. It is okey, cause we get the length of the data originally
    if(persist_read_int(I_BLK_PERSIST_KEY) > 0){
      size_ps_blk_acti =
        persist_read_data(persist_read_int(I_BLK_PERSIST_KEY), blk_buf, sizeof(blk_buf));
      dict_write_data(outbox_iter, AMKEY_ACTI, blk_buf, size_ps_blk_acti);
    }

    // get pinteract data. It is okey, cause we get the length of the data originally
    size_ps_blk_pi =
      persist_read_data(PIRPS_B1_PERSIST_KEY, blk_buf, sizeof(blk_buf));
    if(read_res_buf_byte_count(blk_buf, PINTERACT_PS_B_COUNT_IND)
      > PINTERACT_PS_HEAD_B_SIZE){
      dict_write_data(outbox_iter, AMKEY_PINTERACT, blk_buf, size_ps_blk_pi);
    }

    dict_write_end(outbox_iter);
    app_message_outbox_send();
  }
}

static void retry_transmit_data_to_phone(){
  log_codes_transmit(13);
  if(bluetooth_connection_service_peek() && data_to_send()){
    // if there is still data to send, keep doing that
    attempt_transmit_data_to_phone();
  }else if( bluetooth_connection_service_peek()
    && (trans_code_lcl == TR_PUSH_ALL_DATA_TO_SERVER) ){
    // if there is no more data to send, we have bluetooth, and it is a push to
    // server, THEN we attempt to push to server.
    setup_attempt_to_transmit_push_to_server();
    attempt_to_transmit_push_to_server();
  }
}


static void outbox_failed_data_to_phone_cb(DictionaryIterator *iterator, AppMessageResult reason, void *context){
  //do nothing and wait for the next chance to transmit
  log_codes_transmit(6);
  // APP_LOG(APP_LOG_LEVEL_ERROR, "data to phone Transmit dropped!");

  //   window_stack_pop(false);
  if(retry_count < num_retry){
    retry_count += 1; // increase the retry count with each retry
    retry_transmit_data_to_phone();
  }else{
    // if we have no more retries, then we give up and close out
    // reset the retry count once we have finished re-trying to transmit
    retry_count = 0;
    // we can exit the transmission in only two ways. 1. we acknowledge that the
    // message has been recieved, or 2. that the timer runs out.
    // Because, otherwise, we don't know when the last message will be stored
    // in local storage, so we just give it the rest of the time.
  }
}

static void outbox_sent_data_to_phone_cb(DictionaryIterator *iterator, void *context){
  // this is not a reliable measure of success, so we put this code in the
  // section that responds to appmessages from the phone
}

static void setup_attempt_transmit_data_to_phone(){
  // APP_LOG(APP_LOG_LEVEL_ERROR, "try to open message");
  log_codes_transmit(8);
  // NOTE, the outbox must be zero or else we run out of heap space
  app_message_register_outbox_failed(outbox_failed_data_to_phone_cb);
  app_message_register_outbox_sent(outbox_sent_data_to_phone_cb);
  APP_LOG(APP_LOG_LEVEL_ERROR, "transmit rem heap B %d",(int) heap_bytes_free());
}

// to signal that the phone is ready
static void inbox_received_phone_status_cb(DictionaryIterator *iterator, void *context){
  // set up callbacks and send messages
  Tuple *js_status = dict_find(iterator, AMKEY_JS_STATUS);
  log_codes_transmit(16);
  if(js_status){
    switch(js_status->value->uint8){
      case 1:
        if(data_to_send()){
          // APP_LOG(APP_LOG_LEVEL_ERROR, "got to send data part 1 ");
          log_codes_transmit(10);
          // if we have any data, push to server or no, we do that first, THEN
          // we have a callback to send the data to the server
          setup_attempt_transmit_data_to_phone();
          attempt_transmit_data_to_phone();
        }else if(trans_code_lcl == TR_PUSH_ALL_DATA_TO_SERVER){
          // APP_LOG(APP_LOG_LEVEL_ERROR, "got to push to server part 1 ");
          log_codes_transmit(11);
          // if no data to send, then we must be just trying to push to server
          setup_attempt_to_transmit_push_to_server();
          attempt_to_transmit_push_to_server();
        }
        break;
      case 2:
        // if there is data to send, then we continue to do that until
        if(data_to_send()){
          log_codes_transmit(7);
          // log_codes_transmit(1);
          // we reset the retry counter, because we succeeded in sending a message
          // clean the measures, decrement the actigraphy
          retry_count = 0;
          decrement_legal_I_BLK_PERSIST_KEY();
          // If we have in FACT sent the first message, then we know the pinteract has
          // been sent, so we can reset the pinteract persistant storage.
          reset_pinteract_persistent_storage();
          // log the current block index
          APP_LOG(APP_LOG_LEVEL_ERROR, "iblk: %d",
                  (int16_t) persist_read_int(I_BLK_PERSIST_KEY));
          // send the next actigraphy message.
          retry_transmit_data_to_phone();
        }

        // that if there is NO data to send, and that we are NOT pushing to server,
        // then we close out. If we are not pushing to server, AND there is data
        // then we continue on. If there is NO data And we are pushing to server
        // then we wait for the timer to run out. If we have data AND we are pushing
        // to server (which shouldn't happen), then we close out
        if(!data_to_send() && (!(trans_code_lcl == TR_PUSH_ALL_DATA_TO_SERVER))){
          log_codes_transmit(17);
          close_transmit_window();
        }
        // NOTE : if there is no more data to send, then do nothing
        break;
      case 3:
        // for the case where we have sent all the data to server, we quit out
        log_codes_transmit(18);
        close_transmit_window();
        break;
    }
  }
}

static void inbox_dropped_phone_status_cb(AppMessageResult reason, void *context){
  // if the phone is active, and we drop a message, then simply wait for the
  // timer to run out.
  log_codes_transmit(19);
  // close_transmit_window();
}


// NOTE ! We don't want to give them any options to get out of this.
// WE have to assume that this is going to be alright, and that
// the tick handlers will kill the window when it is time.

static void select_click_handler(ClickRecognizerRef recognizer, void *context){}
static void back_click_handler(ClickRecognizerRef recognizer, void *context){}
static void up_click_handler(ClickRecognizerRef recognizer, void *context){}
static void down_click_handler(ClickRecognizerRef recognizer, void *context){}

static void click_config_provider(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}



static void countdown_timer_handler(void *data){
  // Use a long-lived buffer, so declare static
  static char s_tr_buf[]="100"; // time remaining buffer

  cur_countdown -= 1; // decrement by one
  countdown_active = true;
  if(cur_countdown > 0){
    // write the current countdown to the string buffer
    snprintf(s_tr_buf, sizeof(s_tr_buf), "%d",cur_countdown);
    text_layer_set_text(s_transmit_countdown_text_layer, s_tr_buf);
    app_timer_push_to_server = app_timer_register(1000, countdown_timer_handler,NULL);
  }else{
    countdown_active = false;
    // once reach the end of the countdown, remove the window no matter what
    close_transmit_window();
  }
}


static void transmit_phone_window_load(Window *window) {

  window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  window_set_background_color(window,GColorBlack);
  // push explanantion text
  s_transmit_text_layer = text_layer_create(GRect(5,10,window_bounds.size.w-10,60));
  text_layer_set_text_alignment(s_transmit_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_transmit_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28));
  // change color of text and background
  text_layer_set_background_color(s_transmit_text_layer, GColorBlack);
  text_layer_set_text_color(s_transmit_text_layer, GColorWhite);
  text_layer_set_text(s_transmit_text_layer, "Transmitting\nPlease wait");
  layer_add_child(window_layer, text_layer_get_layer(s_transmit_text_layer));

  // push the countdown text
  s_transmit_countdown_text_layer = text_layer_create(GRect(50,90,45,45));
  text_layer_set_text_alignment(s_transmit_countdown_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_transmit_countdown_text_layer,fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  // change color of text and background
  text_layer_set_background_color(s_transmit_countdown_text_layer, GColorBlack);
  text_layer_set_text_color(s_transmit_countdown_text_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_transmit_countdown_text_layer));

  // since updating time, subscribe to tick timer service
  cur_countdown = NUM_SEC_TRANSMIT_SERVER;
  countdown_timer_handler(NULL);
}


static void transmit_phone_window_unload(Window *window) {
  // get rid of text layer
  text_layer_destroy(s_transmit_text_layer);
  // IF we get to the transmit count down timer, then we destroy those elements
  if(countdown_active){
    app_timer_cancel(app_timer_push_to_server);
  }
  text_layer_destroy(s_transmit_countdown_text_layer);
  // Destroy the banner layer

  // desregister the callbacks LAST
  app_message_deregister_callbacks();
}


// this is triggered not by the wake up but by the worker.
// the wakeups take care of getting everything back up and running
// the worker deals with the other stuff
void init_transmit_to_phone(enum TransmitReason trans_code){
  trans_code_lcl = trans_code;

  // IF we have ANY data to send (even if it is just the flag to transmit
  // to the server) AND the bluetooth is connected, then we ATTEMPT to send
  // the data. NOTE, even if we just pushing to phone, we MUST have a window
  // open for AS LONG as we attempt to retry, so that the outbox callbacks
  // remain registered.
  // 2 cases to proceed, given bluetooth is up
  // 1. we want to only tranmit to the phone, AND we have data
  // 2. we want to push to the server, and we MIGHT have data

  if(bluetooth_connection_service_peek()
    && ( ((trans_code_lcl == TR_PUSH_ALL_DATA_TO_PHONE) && data_to_send())
      || (trans_code_lcl == TR_PUSH_ALL_DATA_TO_SERVER)) ){
    // APP_LOG(APP_LOG_LEVEL_ERROR, "at least trying to transmit to phones");
    log_codes_transmit(9);
    // app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());

    // CREATE THE APP MESSAGE HANDLERS FIRST!!!
    // have the phone app trigger when it is ready
    app_message_register_inbox_received(inbox_received_phone_status_cb);
    app_message_register_inbox_dropped(inbox_dropped_phone_status_cb);

    // open the message buffers. NOTE: should do this AFTER the handlers
    // are registered, in case the ready message makes it early
    // app_message_open(1,app_message_outbox_size_maximum());
    app_message_open(4,1000);

    // THEN WE CREATE THE WINDOW TO START THE READY MESSAGE
    // push the blank transmit phone window to the front ONLY if we have data
    // or to push to server
    s_transmit_phone_window = window_create();
    // app_message_open(app_message_inbox_size_maximum(),app_message_outbox_size_maximum());

    window_set_window_handlers(s_transmit_phone_window, (WindowHandlers) {
      .load = transmit_phone_window_load,
      .unload = transmit_phone_window_unload,
    });

    window_set_click_config_provider(s_transmit_phone_window,
                                     (ClickConfigProvider) click_config_provider);
    #ifdef PBL_SDK_2
      window_set_fullscreen(s_transmit_phone_window,true);
    #endif
    window_stack_push(s_transmit_phone_window, false);

  }
}



// static void setup_countdown_transmit_server(){
//   Layer* window_layer = window_get_root_layer(s_transmit_phone_window);
//
//   // push the countdown text
//   s_transmit_countdown_text_layer = text_layer_create(GRect(50,90,45,45));
//   text_layer_set_text_alignment(s_transmit_countdown_text_layer, GTextAlignmentCenter);
//   text_layer_set_font(s_transmit_countdown_text_layer,fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
//   // change color of text and background
//   text_layer_set_background_color(s_transmit_countdown_text_layer, GColorBlack);
//   text_layer_set_text_color(s_transmit_countdown_text_layer, GColorWhite);
//   layer_add_child(window_layer, text_layer_get_layer(s_transmit_countdown_text_layer));
//
//   // since updating time, subscribe to tick timer service
//   cur_countdown = NUM_SEC_TRANSMIT_SERVER;
//   app_timer_push_to_server = app_timer_register(1000, countdown_timer_handler,NULL);
// }

// int16_t retry_open = 0;
// while((am_result != APP_MSG_OK) && (retry_open < 10)){
//   log_codes_transmit(41);
//   am_result=  app_message_outbox_begin(&outbox_iter);
//   psleep(10);
//   retry_open++;
// }
