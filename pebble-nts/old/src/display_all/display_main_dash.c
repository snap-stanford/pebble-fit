#include "display_func.h"

// HEAP MEMORY USAGE : 508 B

// this function is the main display stats dash, with all UI
// UI functionality. It displays
// 1. percentage of goal as moving circle-like, apple/google standard
// 2. # steps today
// 3. # of kcal today
// ALSO, TIME!


// Simply goals for the flight. Starting at 13:25, we aim to complete
// the main features by 14:30.
// plan
// basic screen cp
// basic
// basic circle drawing (how to)

// constants
// number of angle posistions that are recognized as new points.
// the
static const uint16_t N_ANG_SLOTS = 360/10;

static Window* s_dash_window;

static TextLayer* s_time_layer;
static TextLayer* s_stepc_layer;
static TextLayer* s_stepc_label_layer;
static TextLayer* s_kcal_layer;
static TextLayer* s_kcal_label_layer;
static Layer* s_points_layer;

static uint32_t points; // assume points = kcal
static uint32_t points_goal; // the goal of the points, and endpoint of circle



  // NOTE ! We don't want to give them any options to get out of this.
  // WE have to assume that this is going to be alright, and that
  // the tick handlers will
static void select_click_handler(ClickRecognizerRef recognizer, void *context){

  // DONT CLOSE OUT THE DASH WINDOW!!
  // close out the enter num window entirely
  // window_stack_remove(s_dash_window, false);
  // psleep(50); // wait a bit for the OS to reset the window pointers
  // config_settings_menu();
  // window_stack_remove(s_dash_window, false);
  display_detail_dash();
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context){
  // nothing yet
  #ifdef PBL_COLOR
    window_stack_remove(s_dash_window, false);
    // psleep(300); // wait a bit for the OS to reset the window pointers
    display_pinteract_dots_history(1);
  #endif
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context){
  // nothing yet
  // display_detail_dash();
  #ifdef PBL_COLOR
    window_stack_remove(s_dash_window, false);
    // psleep(100); // wait a bit for the OS to reset the window pointers
    display_pinteract_dots_history(11);
  #endif
  // config_settings_menu();
}

static void click_config_provider(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  // declare the static buffers
  static char time_buf[12];
  static char stepc_buf[10];
  static char kcal_buf[10];
  // update the kcal, steps, and current time

  clock_copy_time_string(time_buf, sizeof(time_buf));
  snprintf(stepc_buf,sizeof(stepc_buf), "%d", (int)persist_read_int(DAILY_STEPC_PERSIST_KEY));
  snprintf(kcal_buf,sizeof(stepc_buf),"%d", (int)persist_read_int(DAILY_x1000_KCAL_PERSIST_KEY)/1000);

  // calculate points
  // points = persist_read_int(DAILY_x1000_KCAL_PERSIST_KEY)/1000;
  // use steps, calories directly would mess people up cause it is for TOTAL day
  points = (int)persist_read_int(DAILY_STEPC_PERSIST_KEY);

  text_layer_set_text(s_time_layer, time_buf);
  text_layer_set_text(s_stepc_layer, stepc_buf);
  text_layer_set_text(s_kcal_layer, kcal_buf);

  // update the moving graphics
  layer_mark_dirty((Layer*) s_time_layer);
  layer_mark_dirty((Layer*) s_stepc_layer);
  layer_mark_dirty((Layer*) s_kcal_layer);
  layer_mark_dirty(s_points_layer);
}

// slot # from points and points_goal
static uint16_t ang_from_points(uint32_t points, uint32_t points_goal){
  return (points < points_goal) ? (points*N_ANG_SLOTS)/points_goal : N_ANG_SLOTS;
}

static void points_layer_update_proc(struct Layer *layer, GContext *ctx){


  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_circle(ctx, GPoint((144/2),(168/2)), (144/2 - 2));
//   graphics_draw_circle(ctx, GPoint((144/2),(168/2)), (144/2 - 20));

  points_goal = get_config_general().pts_goal;

  graphics_context_set_fill_color(ctx, GColorWhite);
  for(int16_t i = 0; i < ang_from_points(points, points_goal); i++){
    graphics_fill_circle(ctx,
      get_point_on_circle_at_ang_CW_noon(
        N_ANG_SLOTS, i, GPoint((144/2),(168/2)), (144/2 - 11)),
      3);
  }

  for(int16_t i = 0; i < N_ANG_SLOTS; i++){
    graphics_fill_circle(ctx,
      get_point_on_circle_at_ang_CW_noon(
        N_ANG_SLOTS, i, GPoint((144/2),(168/2)), (144/2 - 11)),
      1);
  }

}


static void window_load(Window* window){
  // NOTE!!, we are essentially trying to immitate Misfit and jawbone,
  // so white text on black background
  Layer* window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

    // ADD THE MOVING GRAPHICS MUST BE AT BOTTOM OF LAYER STACK
  s_points_layer = layer_create(window_bounds);
  layer_set_update_proc(s_points_layer, points_layer_update_proc);
//   layer_set_background_color(s_points_layer, GColorBlack);
  layer_add_child(window_layer, s_points_layer);

  // seperate out the steps, kcal(points), and time blocks
  int16_t stepc_x = 30;
  int16_t stepc_y = 40;
  int16_t stepc_w = window_bounds.size.w-90;
  int16_t stepc_h = 26;

  int16_t kcal_x = 30;
  int16_t kcal_y = 95;
  int16_t kcal_w = window_bounds.size.w-90;
  int16_t kcal_h = 26;

  // add the time layer
  s_time_layer = text_layer_create(GRect(20,65,window_bounds.size.w-40,30));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));


  // add the stepc
  s_stepc_layer = text_layer_create(GRect(stepc_x,stepc_y,stepc_w,stepc_h));
  text_layer_set_text_alignment(s_stepc_layer, GTextAlignmentRight);
  text_layer_set_font(s_stepc_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_stepc_layer, GColorBlack);
  text_layer_set_text_color(s_stepc_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_stepc_layer));

  // add the stepc label
  s_stepc_label_layer = text_layer_create(GRect(stepc_x + stepc_w + 3,stepc_y + 9,30,15));
  text_layer_set_text_alignment(s_stepc_label_layer, GTextAlignmentLeft);
  text_layer_set_font(s_stepc_label_layer,fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(s_stepc_label_layer, GColorBlack);
  text_layer_set_text_color(s_stepc_label_layer, GColorWhite);
  text_layer_set_text(s_stepc_label_layer, "steps");
  layer_add_child(window_layer, text_layer_get_layer(s_stepc_label_layer));

  // add the kcal
  s_kcal_layer = text_layer_create(GRect(kcal_x,kcal_y,kcal_w,kcal_h));
  text_layer_set_text_alignment(s_kcal_layer, GTextAlignmentRight);
  text_layer_set_font(s_kcal_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_kcal_layer, GColorBlack);
  text_layer_set_text_color(s_kcal_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_kcal_layer));

  // add the kcal label
  s_kcal_label_layer = text_layer_create(GRect(kcal_x + kcal_w + 3,kcal_y+9,30,15));
  text_layer_set_text_alignment(s_kcal_label_layer, GTextAlignmentLeft);
  text_layer_set_font(s_kcal_label_layer,fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(s_kcal_label_layer, GColorBlack);
  text_layer_set_text_color(s_kcal_label_layer, GColorWhite);
  text_layer_set_text(s_kcal_label_layer, "kcal");
  layer_add_child(window_layer, text_layer_get_layer(s_kcal_label_layer));

  time_t init_t = time(NULL);
  // run the tick handler once to update everything
  tick_handler(localtime( &init_t ), SECOND_UNIT);

  APP_LOG(APP_LOG_LEVEL_ERROR, "Main Dash rem heap B: %d",(int) heap_bytes_free());
}

static void window_unload(Window* window){
  // we sure to destroy the moving graphics!!!

  // Destroy all the graphics
  // layer_destroy(s_points_layer);
  // Destroy all the text
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_stepc_layer);
  text_layer_destroy(s_stepc_label_layer);
  text_layer_destroy(s_kcal_layer);
  text_layer_destroy(s_kcal_label_layer);

  // Destroy all the graphics
  layer_destroy(s_points_layer);

  // IMPORTANT !! Unsubscribe the tick time handler
  fore_app_master_tick_timer_service_clock_unsubscribe(SECOND_UNIT);
}

void display_main_dash(){
  // WHENEVER we access the main dash, we just want to clean everything up
  // window_stack_pop_all(false);
  // psleep(100); // NEEDED so OS clears prev window and frees RAM


  // get the points goal from the config file

  s_dash_window = window_create();

  window_set_window_handlers(s_dash_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });

    // cause this function is not defined in basalt, cause it doesn't have status bar
  #ifdef PBL_SDK_2
    window_set_fullscreen(s_dash_window,true);
  #endif

  window_set_click_config_provider(s_dash_window,
                                   (ClickConfigProvider) click_config_provider);

  window_stack_push(s_dash_window,false);

  // SETUP A SECOND TICK HANDLER TO UPDATE THE STEPS!!
  fore_app_master_tick_timer_service_clock_subscribe(SECOND_UNIT, tick_handler);

}
