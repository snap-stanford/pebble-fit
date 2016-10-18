#include "pinteract.h"

// the structure of this file is to call in succession two subinteractions,
// and have them pass on information in the content of a
#include "pinteract.h"
#include "pinteract_structs.h"



// ELEMENT 141
static Window *s_survey_window_141;
static TextLayer *s_row_title_text_layer_141;
static TextLayer *s_num_text_layer_141[2];
static TextLayer *s_label_text_layer_141[2];
static int16_t init_selection_141;

static uint8_t cur_opt_141; // current option
static int32_t cur_num_141[2];
static char num_buf_141[2][8];

// ELEMENT 142
static Window *s_survey_window_142;
static TextLayer *s_title_text_layer_142;
static TextLayer *s_up_text_layer_142;
static TextLayer *s_down_text_layer_142;
static TextLayer *s_num_text_layer_142[5];

static int16_t init_selection_142;
static int16_t cur_opt_142;
static char num_buf_142[5][3];

// Pinteract14Data
Pinteract14Data pi14_res;
static void window_load_142(Window *window);
static void window_unload_142(Window *window);
static void click_config_provider_142(void *context);


// ELEMENT 141

static void select_click_handler_141(ClickRecognizerRef recognizer, void *context){
  // invert the current opt from black background white text to
  text_layer_set_background_color(s_num_text_layer_141[cur_opt_141], GColorWhite);
  text_layer_set_text_color(s_num_text_layer_141[cur_opt_141], GColorBlack);

  cur_opt_141++;
  // if have reached the final opt, then give the data to the finish function
  if(cur_opt_141 == 2){
    // write the new data to the response struct
    pi14_res.sleep_duration_min = cur_num_141[0]*60 + cur_num_141[1];
    window_stack_remove(s_survey_window_141, false);

    // create the next window
    s_survey_window_142 = window_create();
    window_set_window_handlers(s_survey_window_142, (WindowHandlers){
      .load = window_load_142,
      .unload = window_unload_142
    });
    window_set_click_config_provider(s_survey_window_142,
                                     (ClickConfigProvider) click_config_provider_142);
    window_stack_push(s_survey_window_142,false);

  }else{
    // invert the current opt from white background black text
    text_layer_set_background_color(s_num_text_layer_141[cur_opt_141], GColorBlack);
    text_layer_set_text_color(s_num_text_layer_141[cur_opt_141], GColorWhite);
  }

}

static void back_click_handler_141(ClickRecognizerRef recognizer, void *context){

  // invert the current opt from black background white text to
  text_layer_set_background_color(s_num_text_layer_141[cur_opt_141], GColorWhite);
  text_layer_set_text_color(s_num_text_layer_141[cur_opt_141], GColorBlack);
  // push back the element
  cur_opt_141 = (cur_opt_141 > 0) ? cur_opt_141-1 : 0;

  // invert the current opt from white background black text
  text_layer_set_background_color(s_num_text_layer_141[cur_opt_141], GColorBlack);
  text_layer_set_text_color(s_num_text_layer_141[cur_opt_141], GColorWhite);

}

static void up_click_handler_141(ClickRecognizerRef recognizer, void *context){
  // increment the number until reach higher limit

  int32_t high_num_limit = (cur_opt_141==0) ? 23 : 50;
  int32_t num_delta =   (cur_opt_141==0) ? 1 : 10;
  cur_num_141[cur_opt_141] = (cur_num_141[cur_opt_141] < high_num_limit)
    ? (cur_num_141[cur_opt_141] + num_delta) : high_num_limit;
  // update the text layer
  snprintf(num_buf_141[cur_opt_141], sizeof(num_buf_141[cur_opt_141]), "%d",(int)cur_num_141[cur_opt_141]);
  text_layer_set_text(s_num_text_layer_141[cur_opt_141], num_buf_141[cur_opt_141]);
  layer_mark_dirty((Layer*) s_num_text_layer_141[cur_opt_141]);
}


static void down_click_handler_141(ClickRecognizerRef recognizer, void *context){
  // increment the number until reach higher limit
  int32_t low_num_limit = (cur_opt_141==0) ? 0 : 0;
  int32_t num_delta =   (cur_opt_141==0) ? 1 : 10;
  cur_num_141[cur_opt_141] = (cur_num_141[cur_opt_141] > low_num_limit)
    ? (cur_num_141[cur_opt_141] - num_delta) : low_num_limit;

  // update the text layer
  snprintf(num_buf_141[cur_opt_141], sizeof(num_buf_141[cur_opt_141]), "%d",(int)cur_num_141[cur_opt_141]);
  text_layer_set_text(s_num_text_layer_141[cur_opt_141], num_buf_141[cur_opt_141]);
  layer_mark_dirty((Layer*) s_num_text_layer_141[cur_opt_141]);

}

static void click_config_provider_141(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler_141);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler_141);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler_141);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler_141);
}


void window_unload_141(Window *window) {
  // well, we create all the text layers, just the ones we dont' use are pushed
  // off to the side
  text_layer_destroy(s_row_title_text_layer_141);

  for(int16_t i = 0; i < 2; i++){
    text_layer_destroy(s_num_text_layer_141[i]);
    text_layer_destroy(s_label_text_layer_141[i]);
  }
}

static void window_load_141(Window* window){
  cur_opt_141 = 0;
  cur_num_141[0] = init_selection_141/60;
  cur_num_141[1] = init_selection_141%60;
  Layer* window_layer = window_get_root_layer(window);
  // GRect window_bounds = layer_get_bounds(window_layer);
  window_set_background_color(window,GColorWhite);

  // define the relative positions of all the elements
  // dimensions that vary with # of rows
  int16_t rows_x_ofs = 13;
  int16_t rows_title_w = 120;
  int16_t num_label_sep = 2;

  // for 1 row
  int16_t rows1_y_ofs = 9;
  int16_t rows1_title_h = 65;
  char * rows1_title_font = FONT_KEY_GOTHIC_28_BOLD;
  int16_t rows1_num_h = 33;
  char * rows1_num_font = FONT_KEY_GOTHIC_24_BOLD;
  int16_t rows1_label_h = 27;
  char * rows1_label_font = FONT_KEY_GOTHIC_18;

  int16_t cols2_x_ofs = -3;
  int16_t cols2_num_w = 33;
  int16_t cols2_label_w = 24;
  int16_t cols_sep = cols2_num_w + num_label_sep + cols2_label_w + 9;

  // static TextLayer *s_row_title_text_layer_141;
  // static TextLayer *s_num_text_layer_141[2];
  // static TextLayer *s_label_text_layer_141[2];

  // TITLE
  s_row_title_text_layer_141 =
    text_layer_create(GRect(rows_x_ofs,rows1_y_ofs,rows_title_w,rows1_title_h));
  text_layer_set_font(s_row_title_text_layer_141,fonts_get_system_font(rows1_title_font));
  text_layer_set_text(s_row_title_text_layer_141,"Sleep Duration");
  text_layer_set_text_alignment(s_row_title_text_layer_141, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_row_title_text_layer_141));


  // COLUMN 0
  s_num_text_layer_141[0] =
    text_layer_create(GRect(
      rows_x_ofs + cols2_x_ofs,
      rows1_y_ofs + rows1_title_h,
      cols2_num_w, rows1_num_h));
  text_layer_set_font(s_num_text_layer_141[0],fonts_get_system_font(rows1_num_font));
  // set the text and add the layer
  snprintf(num_buf_141[0], sizeof(num_buf_141[0]), "%d",(int) cur_num_141[0]);
  text_layer_set_text(s_num_text_layer_141[0], num_buf_141[0]);
  text_layer_set_text_alignment(s_num_text_layer_141[0], GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_num_text_layer_141[0]));


  s_label_text_layer_141[0] =
    text_layer_create(GRect(
      rows_x_ofs + cols2_x_ofs + cols2_num_w + num_label_sep,
      rows1_y_ofs + rows1_title_h + (rows1_num_h - rows1_label_h),
      cols2_label_w, rows1_label_h));
  text_layer_set_font(s_label_text_layer_141[0],fonts_get_system_font(rows1_label_font));
  text_layer_set_text(s_label_text_layer_141[0], "hrs");
  text_layer_set_text_alignment(s_label_text_layer_141[0], GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_label_text_layer_141[0]));


  // COLUMN 1
  s_num_text_layer_141[1] =
    text_layer_create(GRect(
      rows_x_ofs + cols2_x_ofs + cols_sep,
      rows1_y_ofs + rows1_title_h,
      cols2_num_w, rows1_num_h));
  text_layer_set_font(s_num_text_layer_141[1],fonts_get_system_font(rows1_num_font));
  snprintf(num_buf_141[1], sizeof(num_buf_141[1]), "%d",(int) cur_num_141[1]);
  text_layer_set_text(s_num_text_layer_141[1], num_buf_141[1]);
  text_layer_set_text_alignment(s_num_text_layer_141[1], GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_num_text_layer_141[1]));


  s_label_text_layer_141[1] =
    text_layer_create(GRect(
      rows_x_ofs + cols2_x_ofs + cols_sep + cols2_num_w + num_label_sep ,
      rows1_y_ofs + rows1_title_h + (rows1_num_h - rows1_label_h),
      cols2_label_w, rows1_label_h));
  text_layer_set_font(s_label_text_layer_141[1],fonts_get_system_font(rows1_label_font));
  text_layer_set_text(s_label_text_layer_141[1], "min");
  text_layer_set_text_alignment(s_label_text_layer_141[1], GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_label_text_layer_141[1]));


  text_layer_set_background_color(s_num_text_layer_141[cur_opt_141], GColorBlack);
  text_layer_set_text_color(s_num_text_layer_141[cur_opt_141], GColorWhite);

}


// ELEMENT 142
static void select_click_handler_142(ClickRecognizerRef recognizer, void *context){
  // invert the current opt from black background white text to
  pi14_res.time_end_pi = time(NULL);
  pi14_res.sleep_quality = 5 - cur_opt_142;

  // SET THE NEW PINTERACT STATE
  Pinteract14State pi14_state ={
    .sleep_duration_min = pi14_res.sleep_duration_min,
    .sleep_quality_index = cur_opt_142
  };
  set_pinteract_state(14, (void*) &pi14_state, 0);

  int16_t new_pinteract_count = persist_read_int(PINTERACT_KEY_COUNT_PERSIST_KEY) + 1;
  // write the new data to persistant storage
  persist_write_data(new_pinteract_count, &pi14_res, sizeof(Pinteract14Data) );
  // update the pinteract storage count
  persist_write_int(PINTERACT_KEY_COUNT_PERSIST_KEY, new_pinteract_count);

  window_stack_remove(s_survey_window_142, false);

}

static void back_click_handler_142(ClickRecognizerRef recognizer, void *context){
  window_stack_remove(s_survey_window_142, false);
  s_survey_window_141 = window_create();
  window_set_window_handlers(s_survey_window_141, (WindowHandlers){
    .load = window_load_141,
    .unload = window_unload_141
  });
  window_set_click_config_provider(s_survey_window_141,
                                   (ClickConfigProvider) click_config_provider_141);
  window_stack_push(s_survey_window_141,false);

}

static void up_click_handler_142(ClickRecognizerRef recognizer, void *context){
  text_layer_set_background_color(s_num_text_layer_142[cur_opt_142], GColorWhite);
  text_layer_set_text_color(s_num_text_layer_142[cur_opt_142], GColorBlack);
  // decrement the current option until you have the 0 index
  cur_opt_142 = (cur_opt_142>0) ? cur_opt_142 - 1 : 0;
  text_layer_set_background_color(s_num_text_layer_142[cur_opt_142], GColorBlack);
  text_layer_set_text_color(s_num_text_layer_142[cur_opt_142], GColorWhite);
}


static void down_click_handler_142(ClickRecognizerRef recognizer, void *context){
  text_layer_set_background_color(s_num_text_layer_142[cur_opt_142], GColorWhite);
  text_layer_set_text_color(s_num_text_layer_142[cur_opt_142], GColorBlack);
  // decrement the current option until you have the 0 index
  cur_opt_142 = (cur_opt_142<4) ? cur_opt_142 + 1 : 4;
  text_layer_set_background_color(s_num_text_layer_142[cur_opt_142], GColorBlack);
  text_layer_set_text_color(s_num_text_layer_142[cur_opt_142], GColorWhite);
}

static void click_config_provider_142(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler_142);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler_142);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler_142);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler_142);
}


static void window_load_142(Window *window){

  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  cur_opt_142 = init_selection_142;

  // static TextLayer *s_title_text_layer_142;
  // static TextLayer *s_up_text_layer_142;
  // static TextLayer *s_down_text_layer_142;
  // static TextLayer *s_num_text_layer_142[5];

  int16_t ROW_HEIGHT = 32;
  int16_t MENU_WIDTH = 20;
  int16_t MENU_LEFT_PX = window_bounds.size.w - MENU_WIDTH ;
  int16_t OPTION_LEFT_PX = 2;

  // add the title text layer
  int16_t TITLE_HEIGHT = 75;
  s_title_text_layer_142 = text_layer_create( GRect(5,50,MENU_LEFT_PX - 5 ,TITLE_HEIGHT));
  text_layer_set_text_alignment(s_title_text_layer_142, GTextAlignmentLeft);
  text_layer_set_font(s_title_text_layer_142,fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_title_text_layer_142,"Sleep\nQuality?");
  layer_add_child(window_layer,text_layer_get_layer(s_title_text_layer_142));


  // Create and add the up_option text
  s_up_text_layer_142 = text_layer_create(
    GRect(OPTION_LEFT_PX,2, MENU_LEFT_PX-OPTION_LEFT_PX,ROW_HEIGHT+1));
  text_layer_set_text_alignment(s_up_text_layer_142, GTextAlignmentRight);
  text_layer_set_font(s_up_text_layer_142,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_up_text_layer_142, "Very Good  ");
  text_layer_set_background_color(s_up_text_layer_142, GColorBlack);
  text_layer_set_text_color(s_up_text_layer_142, GColorWhite);
  layer_add_child(window_layer,text_layer_get_layer(s_up_text_layer_142));

  // Create and add the down_option text
  s_down_text_layer_142 = text_layer_create(
    GRect(OPTION_LEFT_PX,window_bounds.size.h -ROW_HEIGHT-3, MENU_LEFT_PX-OPTION_LEFT_PX, ROW_HEIGHT+1));
  text_layer_set_text_alignment(s_down_text_layer_142, GTextAlignmentRight);
  text_layer_set_font(s_down_text_layer_142, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_down_text_layer_142, "Very Poor  ");
  text_layer_set_background_color(s_down_text_layer_142, GColorBlack);
  text_layer_set_text_color(s_down_text_layer_142, GColorWhite);
  layer_add_child(window_layer,text_layer_get_layer(s_down_text_layer_142));

  // add the number text layers
  // divide the screen into 5 equal sections
  int16_t NUM_LAYER_HEIGHT = window_bounds.size.h/5;
  for(int16_t i = 0; i< 5 ; i++){
    s_num_text_layer_142[i] = text_layer_create(
      GRect(MENU_LEFT_PX,NUM_LAYER_HEIGHT*i+2, MENU_WIDTH, NUM_LAYER_HEIGHT));
      text_layer_set_text_alignment(s_num_text_layer_142[i], GTextAlignmentCenter);
      text_layer_set_font(s_num_text_layer_142[i], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    // write the number to the buffer
    snprintf(num_buf_142[i], sizeof(num_buf_142[i]), "%d",(int)(5-i));
    text_layer_set_text(s_num_text_layer_142[i], num_buf_142[i]);
    // set the text layers to the right color, excepting the inital selection
    if(i != cur_opt_142){
      text_layer_set_background_color(s_num_text_layer_142[i], GColorWhite);
      text_layer_set_text_color(s_num_text_layer_142[i], GColorBlack);
    }else{
      text_layer_set_background_color(s_num_text_layer_142[i], GColorBlack);
      text_layer_set_text_color(s_num_text_layer_142[i], GColorWhite);
    }
    // add the number text layer to the page
    layer_add_child(window_layer,text_layer_get_layer(s_num_text_layer_142[i]));
  }

  // see what the hell is going on with the heap amount
  APP_LOG(APP_LOG_LEVEL_ERROR, "heap size: used %d , free %d", heap_bytes_used(), heap_bytes_free());
}


static void window_unload_142(Window *window){
  // destroy the  text layers
  text_layer_destroy(s_title_text_layer_142);
  text_layer_destroy(s_up_text_layer_142);
  text_layer_destroy(s_down_text_layer_142);
  for(int16_t i = 0; i < 5;i++){
    text_layer_destroy(s_num_text_layer_142[i]);
  }
  window_destroy(window);
}


void pinteract_14(PinteractContext ctx){

  pi14_res.time_srt_priv_scrn = ctx.time_srt_priv_scrn;
  pi14_res.time_srt_pi = time(NULL); // time that we start the p interaction
  pi14_res.pinteract_code = 14;
  pi14_res.data_size = sizeof(Pinteract14Data);

  // initialize the local variables
  int16_t prev_state_141 = get_pinteract_state_all().pi_14[1].sleep_duration_min;
  int8_t prev_state_142 = get_pinteract_state_all().pi_14[1].sleep_quality_index;
  init_selection_141 = (prev_state_141 != -1 && prev_state_141 <= 24*60 &&  prev_state_141 >= 0 ) ? prev_state_141 : 8*60;
  init_selection_142 = (prev_state_142 != -1 && prev_state_142 <= 4 && prev_state_142 >= 0) ? prev_state_142 : 2;

  // create the window
  s_survey_window_141 = window_create();
  window_set_window_handlers(s_survey_window_141, (WindowHandlers){
    .load = window_load_141,
    .unload = window_unload_141
  });

  window_set_click_config_provider(s_survey_window_141,
                                   (ClickConfigProvider) click_config_provider_141);
  window_stack_push(s_survey_window_141,false);
}
