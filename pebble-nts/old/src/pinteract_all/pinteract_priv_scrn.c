#include "pinteract_func.h"

// STATIC RAM MEMORY USAGE : 902 Bytes
// HEAP MEMORY USAGE : 1124 Bytes

// this is the patient privacy screen that they must always have
// to click through.

// Basic actions
// 1. present a screen
//    - note when it was displayed, and place a code and a timestamp
//     using simple struct to byte converstion. A convience method for
//     appending byte arrays to the end of a larger one with active subset
//     is needed.
// 2. when patient clicks next button, goes to next interaction
//

// NOTE: Pebble screen is 144x168


static Window *s_privacy_window;
static TextLayer *s_time_layer; // time written over static graphics layer
static TextLayer *s_up_delay_layer; // time written over static graphics layer
static TextLayer *s_down_delay_layer; // time written over static graphics layer

static BitmapLayer *s_question_bitmap_layer;
static BitmapLayer *s_select_arrow_bitmap_layer;

static GBitmap *s_question_bitmap;
static GBitmap *s_select_arrow_bitmap;

// also, we can make life easier if we just have globals local
// to this function that
static struct pinteract_func_ll_el* pif_ll_el_lcl;
static time_t time_t_srt_priv_scrn_lcl;
static uint16_t this_pinteract_code = 1;

static uint16_t up_delay_mins = 30;
static uint16_t down_delay_hrs = 3;
static time_t reminder_buzz_interval = 1*60*60; // 1*60*60, = 1 hours
static time_t reminder_buzz_time_t;


// Click Handlers

// Back button, nothing
// up button, nothing
// down button, nothing

// select button, one click, calls window_unload


static void finish_this_interact(){

  // write the mood rating to the buffer
  uint8_t * res_buf = pif_ll_el_lcl->buf;

  struct pinteract_priv_scrn_res pi_s_res = {
    .time_t_srt_priv_scrn = time_t_srt_priv_scrn_lcl,
    .time_t_srt_pi = time(NULL),
    .pinteract_code = this_pinteract_code
  };

  write_data_to_res_buf(res_buf, (uint8_t*) &pi_s_res, sizeof(pi_s_res));

  // call the next function
  (pif_ll_el_lcl->pif)(pif_ll_el_lcl->next);

}

static void close_priv_scrn_window(){
  window_stack_remove(s_privacy_window, false);
  psleep(100);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  // SO, The convention is that the "calling the next interaction" happens
  // in the main_window_unload function, always. That way, we always know
  // that the window is undone,and that calling the next function doesn't
  // mess with the current interaction
  close_priv_scrn_window();

  // write the data to the response buffer
  finish_this_interact();
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context){
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context){
  // delay 10 mins

  time_t wakeup_time_t = time(NULL) + 60*(up_delay_mins); // minutes;
  reschedule_config_wakeup_index(
    persist_read_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY), wakeup_time_t);
  // NOTE!!, we can set ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY to -1 because the
  // wakeup has already been set, and that when the wakeup occurs the
  // ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY will again be set to the correct config_i
  persist_write_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY,-1);

  // remove the current window
  close_priv_scrn_window();

}

static void down_click_handler(ClickRecognizerRef recognizer, void *context){
  // delay 2 hours

  // reschedule the time using the schedule config function and
  time_t wakeup_time_t = time(NULL) + 60*(60)*(down_delay_hrs); // hours;
  reschedule_config_wakeup_index(
    persist_read_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY), wakeup_time_t);
  // NOTE!!, we can set ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY to -1 because the
  // wakeup has already been set, and that when the wakeup occurs the
  // ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY will again be set to the correct config_i
  persist_write_int(ACTIVE_WAKEUP_CONFIG_I_PERSIST_KEY,-1);

  // remove the current window
  close_priv_scrn_window();
}


static void click_config_provider(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  // Use a long-lived buffer, so declare static
  static char s_time_buffer[10];

  // Update the TextLayer
  // strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  clock_copy_time_string(s_time_buffer, sizeof(s_time_buffer));
  text_layer_set_text(s_time_layer, s_time_buffer);

  // vibrate to remind the person to input if reach a time limit
  if( time(NULL) > reminder_buzz_time_t){
    vibes_enqueue_custom_pattern(pinteract_vibe_pat);
    // reset the timer for the NEXT reminder time.
    reminder_buzz_time_t = time(NULL) + reminder_buzz_interval;
  }
}


static void main_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  // change background color
  window_set_background_color(window,GColorBlack);

  // get the bitmap
  s_question_bitmap = gbitmap_create_with_resource(RESOURCE_ID_QUESTION_MARK_64);
  s_select_arrow_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOUBLE_ARROW_RIGHT_24);
  // Create the output Graphics layers
  // s_question_bitmap_layer = bitmap_layer_create(GRect(40,15,64,64));
  s_question_bitmap_layer = bitmap_layer_create(GRect(20,60,64,64));
  s_select_arrow_bitmap_layer = bitmap_layer_create(GRect(113,80,24,24));
  // add the bitmaps to the layers
  bitmap_layer_set_bitmap(s_question_bitmap_layer,s_question_bitmap);
  bitmap_layer_set_bitmap(s_select_arrow_bitmap_layer,s_select_arrow_bitmap);
  // add bitmap layers to the screen
  layer_add_child(window_layer,bitmap_layer_get_layer(s_question_bitmap_layer));
  layer_add_child(window_layer,bitmap_layer_get_layer(s_select_arrow_bitmap_layer));


  // Create the UP and DOWN delay options Text Layers
  int8_t delay_opt_h = 30;
  static char s_up_delay_buf[17];
  s_up_delay_layer = text_layer_create(GRect(0,0,window_bounds.size.w,delay_opt_h));
  text_layer_set_text_alignment(s_up_delay_layer, GTextAlignmentRight);
  text_layer_set_font(s_up_delay_layer,fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_background_color(s_up_delay_layer, GColorBlack);
  text_layer_set_text_color(s_up_delay_layer, GColorWhite);
  snprintf(s_up_delay_buf, sizeof(s_up_delay_buf), "Delay %d min > ",up_delay_mins);
  text_layer_set_text(s_up_delay_layer, s_up_delay_buf);

  static char s_down_delay_buf[17];
  s_down_delay_layer = text_layer_create(GRect(0,168-delay_opt_h+3,window_bounds.size.w,delay_opt_h));
  text_layer_set_text_alignment(s_down_delay_layer, GTextAlignmentRight);
  text_layer_set_font(s_down_delay_layer,fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_background_color(s_down_delay_layer, GColorBlack);
  text_layer_set_text_color(s_down_delay_layer, GColorWhite);
  snprintf(s_down_delay_buf, sizeof(s_down_delay_buf), "Delay %d hrs > ",down_delay_hrs);
  text_layer_set_text(s_down_delay_layer, s_down_delay_buf);

  // add the up down delay layers
  layer_add_child(window_layer, text_layer_get_layer(s_up_delay_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_down_delay_layer));


  // Create the TIME Text Layer
  s_time_layer = text_layer_create(GRect(1,25,100,32));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  // change color of text and background
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);

  // update the time before adding
  time_t cur_time = time(NULL);
  struct tm *tick_time = localtime(&cur_time);
  tick_handler(tick_time, MINUTE_UNIT);
  // later change this to be a child of the Graphics layer?
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  APP_LOG(APP_LOG_LEVEL_ERROR, "Priv scn heap size: used %d , free %d", heap_bytes_used(), heap_bytes_free());

}


static void main_window_unload(Window *window){

  // destoy the bitmaps
  gbitmap_destroy(s_question_bitmap);
  gbitmap_destroy(s_select_arrow_bitmap);
  bitmap_layer_destroy(s_question_bitmap_layer);
  bitmap_layer_destroy(s_select_arrow_bitmap_layer);
  // remove the text layers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_up_delay_layer); // time written over static graphics layer
  text_layer_destroy(s_down_delay_layer); // time written over static graphics layer

  fore_app_master_tick_timer_service_aux_unsubscribe(MINUTE_UNIT);

}

// This function acts precisely as the initizalizer. And, like other mood view,
// we avoid the use of a header file by declaring functions in order of appearance
// unless unavoiable.

uint16_t pinteract_priv_scrn(struct pinteract_func_ll_el* pif_ll_el ){

  // START to build the privacy window
  reminder_buzz_time_t = time(NULL) + reminder_buzz_interval;

  // get rid of everything, this is the most important thing there is, even
  // more important than any transmission, even more important than previous
  // privacy screens or previous interactions
  // SO!!, we will make it happen, regardless
  APP_LOG(APP_LOG_LEVEL_ERROR, "Before clear stack, priv scrn rem heap B: %d",(int) heap_bytes_free());
  window_stack_pop_all(false);
  // psleep(200); // NEEDED so OS clears prev window and frees RAM
  APP_LOG(APP_LOG_LEVEL_ERROR, "After clear stack, priv scrn, rem heap B: %d",(int) heap_bytes_free());


  pif_ll_el_lcl = pif_ll_el;
  time_t_srt_priv_scrn_lcl = time(NULL);

  // create window
  s_privacy_window = window_create();

  // use global variables to this file for sanity
  // set the window handlers
  window_set_window_handlers(s_privacy_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload,
  });
  // Set the click handlers
  window_set_click_config_provider(s_privacy_window, (ClickConfigProvider) click_config_provider);

    // cause this function is not defined in basalt, cause it doesn't have status bar
  #ifdef PBL_SDK_2
    window_set_fullscreen(s_privacy_window,true);
  #endif

  window_stack_push(s_privacy_window, true);
  // since updating time, subscribe to tick timer service
  fore_app_master_tick_timer_service_aux_subscribe(MINUTE_UNIT, tick_handler);

  // vibrate to remind the person to input
  vibes_enqueue_custom_pattern(pinteract_vibe_pat);

  // guarding if a pinteraction will write too much? Probably get rid of it
  return this_pinteract_code; // we have written nothing to it here
}
