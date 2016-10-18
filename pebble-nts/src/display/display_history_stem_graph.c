// this is a file that simply displays the last 10 days of mood data

// be sure to write into the background worker to move the day over at midnight
#include "display.h"



static Window* s_history_stem_window;
static Layer* s_dots_layer;
static TextLayer* s_title_layer;
// static TextLayer* s_dates_layer;
// static TextLayer* s_day_layer;
static TextLayer* s_axis_y_label_layer;
static TextLayer* s_wday_layers[7];


static int16_t num_bars;
static int16_t bar_w;
static int16_t mid_y;
static int16_t step_h;
// create the axis
static int16_t axis_x;


// static int32_t min_kcal;


// text layer for title :  "mood history"
static int16_t history_pi_code;
static int16_t day_offset;


static void set_title_dates_y_axis_label(){
  switch(history_pi_code){
    case 11:
      text_layer_set_text(s_title_layer, "Mood");
      text_layer_set_text(s_axis_y_label_layer, "");
      layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-52,mid_y-23,50,21));
      break;

    case 142:
      text_layer_set_text(s_title_layer, "Sleep Quality");
      text_layer_set_text(s_axis_y_label_layer, "");
      layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-52,mid_y-23,50,21));
      break;
  }
  layer_mark_dirty((Layer *)s_axis_y_label_layer);
  layer_mark_dirty((Layer *)s_title_layer);

}

static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  day_offset = 1;
  layer_mark_dirty(s_dots_layer);
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context){
  switch(history_pi_code){
    case 11 :
      history_pi_code = 142 ;
      break;
    case 142 :
      history_pi_code = 11 ;
      break;
  }

  set_title_dates_y_axis_label();
  layer_mark_dirty(s_dots_layer);

}

static void down_click_handler(ClickRecognizerRef recognizer, void *context){
  // psleep(100); // wait a bit for the OS to reset the window pointers
  switch(history_pi_code){
    case 11 :
      history_pi_code = 142 ;
      break;
    case 142 :
      history_pi_code = 11 ;
      break;
  }
  set_title_dates_y_axis_label();
  layer_mark_dirty(s_dots_layer);
}


static void back_click_handler(ClickRecognizerRef recognizer, void *context){
  // if(day_offset == 1){
  //   day_offset = 8;
  //   text_layer_set_text(s_day_layer, "14");
  //   layer_mark_dirty(s_dots_layer);
  // }else{
  //   window_stack_remove(s_history_stem_window, false);
  //   psleep(100); // wait a bit for the OS to reset the window pointers
  //   display_main_dash();
  // }
  window_stack_remove(s_history_stem_window, false);
}

static void click_config_provider(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}


static void dots_layer_update_proc(struct Layer *layer, GContext *ctx){

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 3);

  switch(history_pi_code){
    case 11:
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,mid_y - step_h*0),
        GPoint((num_bars-1)*bar_w + bar_w/2 +5,mid_y - step_h*0));
      for(int16_t i = 0; i < num_bars; i++){
        int8_t state = get_pinteract_state_all().pi_11[i+day_offset].mood_index;
        if( (state >= 0)){
          int16_t circle_y = mid_y + (state -2)*step_h;

          graphics_context_set_fill_color(ctx, GColorWhite);
          graphics_draw_line(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y),
            GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,mid_y));

          graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y), bar_w/2-4);
        }
      }
      break;
    case 142:
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,mid_y - step_h*0),
        GPoint((num_bars-1)*bar_w + bar_w/2 +5,mid_y - step_h*0));
      for(int16_t i = 0; i < num_bars; i++){
        // note, -1 means the 14 didnot occur that day
        int8_t state = get_pinteract_state_all().pi_14[i+day_offset].sleep_quality_index;
        if( (state >= 0)){
          int16_t circle_y = mid_y + (state -2)*step_h;
          graphics_context_set_fill_color(ctx, GColorWhite);
          graphics_draw_line(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y),
            GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,mid_y));
          graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y), bar_w/2-4);
        }
      }
      break;
  }
}

static void window_load(Window* window){
  // NOTE!!, we are essentially trying to immitate Misfit and jawbone,
  // so white text on black background
  Layer* window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  time_t init_t = time(NULL);
  // run the tick handler once to update everything
  struct tm *tick_time = localtime( &init_t );
  num_bars = 7;
  axis_x = window_bounds.size.w -1;
  bar_w = ((window_bounds.size.w*23/24 )/num_bars);
  // // pinteract 11 & 1
  mid_y = window_bounds.size.h/2 + 5;
  step_h = (window_bounds.size.h*10/12)/5;

  // add title layer
  s_title_layer = text_layer_create(GRect(5,1,window_bounds.size.w-5,21));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentLeft);
  text_layer_set_font(s_title_layer,fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_background_color(s_title_layer, GColorBlack);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  // add the y axis label
  s_axis_y_label_layer = text_layer_create(GRect(axis_x-41,mid_y-21,40,21));
  text_layer_set_text_alignment(s_axis_y_label_layer, GTextAlignmentRight);
  text_layer_set_font(s_axis_y_label_layer,fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_background_color(s_axis_y_label_layer, GColorBlack);
  text_layer_set_text_color(s_axis_y_label_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_axis_y_label_layer));
  set_title_dates_y_axis_label();

  char* weekday[7];
  weekday[0] = "Su";
  weekday[1] = "Mo";
  weekday[2] = "Tu";
  weekday[3] = "We";
  weekday[4] = "Th";
  weekday[5] = "Fr";
  weekday[6] = "Sa";

  for(int8_t i = 0; i < num_bars; i++){
    s_wday_layers[i] = text_layer_create(GRect( (num_bars-1-i)*bar_w + 5 ,
      window_bounds.size.h-18,20,18) );
    text_layer_set_text_alignment(s_wday_layers[i], GTextAlignmentCenter);
    text_layer_set_font(s_wday_layers[i],fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_background_color(s_wday_layers[i], GColorBlack);
    text_layer_set_text_color(s_wday_layers[i], GColorWhite);
    text_layer_set_text(s_wday_layers[i], weekday[ (tick_time->tm_wday -i -1+7)%7 ]);
    layer_add_child(window_layer, text_layer_get_layer(s_wday_layers[i]));
  }

  // ADD THE MOVING GRAPHICS MUST BE AT TOP OF LAYER STACK
  s_dots_layer = layer_create(window_bounds);
  layer_set_update_proc(s_dots_layer, dots_layer_update_proc);
  //   layer_set_background_color(s_points_layer, GColorBlack);
  layer_add_child(window_layer, s_dots_layer);
  layer_mark_dirty(s_dots_layer);
  APP_LOG(APP_LOG_LEVEL_ERROR, "window load : heap size: used %d , free %d",
      heap_bytes_used(), heap_bytes_free());
}


static void window_unload(Window* window){
  // we sure to destroy the moving graphics!!!

  // Destroy all the graphics
  layer_destroy(s_dots_layer);
  text_layer_destroy(s_title_layer);
  // text_layer_destroy(s_dates_layer);
  // text_layer_destroy(s_day_layer);
  text_layer_destroy(s_axis_y_label_layer);

  for(int16_t i = 0; i < 7; i++){
    text_layer_destroy(s_wday_layers[i]);
  }
  window_destroy(window);
  APP_LOG(APP_LOG_LEVEL_ERROR, "window unload : heap size: used %d , free %d",
      heap_bytes_used(), heap_bytes_free());
}

void display_history_stem_graph(int16_t history_pi_code_in){
  APP_LOG(APP_LOG_LEVEL_ERROR, "window start : heap size: used %d , free %d",
      heap_bytes_used(), heap_bytes_free());
  history_pi_code = history_pi_code_in;
  day_offset = 1;
  // WHENEVER we access the main dash, we just want to clean everything up
  // get the points goal from the config file
  s_history_stem_window = window_create();

  window_set_window_handlers(s_history_stem_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });

  window_set_click_config_provider(s_history_stem_window,
                                   (ClickConfigProvider) click_config_provider);

  window_stack_push(s_history_stem_window,false);
}
