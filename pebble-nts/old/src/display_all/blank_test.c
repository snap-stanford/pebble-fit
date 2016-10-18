#include "display_func.h"



static Window* s_blank_test_window;
static TextLayer* s_title_layer;



static void select_click_handler(ClickRecognizerRef recognizer, void *context){
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context){
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context){
  window_stack_remove(s_blank_test_window, false);
  // psleep(100); // wait a bit for the OS to reset the window pointers
  display_main_dash();
}


static void back_click_handler(ClickRecognizerRef recognizer, void *context){

  window_stack_remove(s_blank_test_window, false);
  // psleep(100); // wait a bit for the OS to reset the window pointers
  display_main_dash();
}

static void click_config_provider(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

static void window_load(Window* window){
  // NOTE!!, we are essentially trying to immitate Misfit and jawbone,
  // so white text on black background
  Layer* window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  // add title layer
  s_title_layer = text_layer_create(GRect(20,-2,window_bounds.size.w-40,27));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_title_layer, GColorBlack);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_text(s_title_layer, "Steps");
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));


  //   layer_set_background_color(s_points_layer, GColorBlack);
  APP_LOG(APP_LOG_LEVEL_ERROR, "window load : heap size: used %d , free %d",
      heap_bytes_used(), heap_bytes_free());
}


static void window_unload(Window* window){
  // we sure to destroy the moving graphics!!!
  text_layer_destroy(s_title_layer);
  // text_layer_destroy(s_day_layer);

  APP_LOG(APP_LOG_LEVEL_ERROR, "window unload : heap size: used %d , free %d",
      heap_bytes_used(), heap_bytes_free());
  // psleep(100);
}

void blank_test(){
  APP_LOG(APP_LOG_LEVEL_ERROR, "window start : heap size: used %d , free %d",
      heap_bytes_used(), heap_bytes_free());

  // WHENEVER we access the main dash, we just want to clean everything up
  // window_stack_pop_all(false);
  // psleep(100); // NEEDED so OS clears prev window and frees RAM

  // get the points goal from the config file
  s_blank_test_window = window_create();

  window_set_window_handlers(s_blank_test_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });

    // cause this function is not defined in basalt, cause it doesn't have status bar
  #ifdef PBL_SDK_2
    window_set_fullscreen(s_blank_test_window,true);
  #endif

  window_set_click_config_provider(s_blank_test_window,
                                   (ClickConfigProvider) click_config_provider);

  window_stack_push(s_blank_test_window,false);
}
