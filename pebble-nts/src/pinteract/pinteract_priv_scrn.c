// this page is usually where all pinteracts start, at least ones that are prompted
// by the app itself. Pinteracts that the user chooses can bypass this screen

#include "pinteract.h"

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
static time_t time_t_srt_priv_scrn_lcl = 0;
static uint16_t pinteract_code_lcl;

static uint16_t up_delay_mins = 30;
static uint16_t down_delay_mins = 3*60;
static time_t reminder_buzz_interval = 30*60; // = 30 min
static time_t reminder_buzz_time_t;


static void close_priv_scrn_window(){
  tick_timer_service_unsubscribe();
  window_stack_remove(s_privacy_window, false);
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  // SO, The convention is that the "calling the next interaction" happens
  // in the main_window_unload function, always. That way, we always know
  // that the window is undone,and that calling the next function doesn't
  // mess with the current interaction

  close_priv_scrn_window();

  // write the pinteract context to the pinteract driver
  PinteractContext ctx = {
    .time_srt_priv_scrn = time_t_srt_priv_scrn_lcl
  };
  pinteract_driver(pinteract_code_lcl, ctx);
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
  time_t wakeup_time_t = time(NULL) + 60*(down_delay_mins); // hours;
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
  layer_mark_dirty((Layer *)s_time_layer);

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
  snprintf(s_down_delay_buf, sizeof(s_down_delay_buf), "Delay %d hrs > ",down_delay_mins/60);
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
  window_destroy(window);

  // fore_app_master_tick_timer_service_aux_unsubscribe(MINUTE_UNIT);
  tick_timer_service_unsubscribe();
}

void pinteract_priv_scrn(int16_t pinteract_code){
  // if the window is on the stack, remove it
  if(window_stack_contains_window(s_privacy_window)){
    window_stack_remove(s_privacy_window, false);
  }

  pinteract_code_lcl = pinteract_code;
  // get the start time
  time_t_srt_priv_scrn_lcl = time(NULL);
  reminder_buzz_time_t = reminder_buzz_interval + time(NULL);
  // set up the screen
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
  window_stack_push(s_privacy_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // vibrate to remind the person to input
  vibes_enqueue_custom_pattern(pinteract_vibe_pat);
}
