// be sure to write into the background worker to move the day over at midnight
#include "display_func.h"


static Window* s_acti_history_window;
static Layer* s_bars_layer;
static TextLayer* s_title_layer;
static TextLayer* s_day_layer;

static TextLayer* s_wday_layers[7];

static int16_t num_bars;
static int16_t bar_w;
static int16_t mid_y;
static int16_t step_h;

// create the axis
static int16_t axis_x;

// text layer for title :  "mood history"
static int16_t history_acti_type;
static int16_t day_offset;


static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  day_offset = 1;
  layer_mark_dirty(s_bars_layer);
  text_layer_set_text(s_day_layer, "7");
  // DONT CLOSE OUT THE DASH WINDOW!!
  // display_detail_dash();
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context){
  // nothing yet
  // #ifdef PBL_COLOR
  //   display_pinteract_circle_hist(history_pi_code);
  // #endif
  // if(history_acti_type == 1){
  //   window_stack_remove(s_acti_history_window, false);
  //   psleep(100); // wait a bit for the OS to reset the window pointers
  //   display_acti_bars_history(2);
  // }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context){
  // nothing yet
  // window_stack_remove(s_acti_history_window, false);
  // psleep(100); // wait a bit for the OS to reset the window pointers
  // if(history_acti_type == 1){
  //   display_pinteract_dots_history(11);
  // }else if(history_acti_type == 2){
  //   display_acti_bars_history(1);
  // }
}


static void back_click_handler(ClickRecognizerRef recognizer, void *context){
  // if(day_offset == 1){
  //   day_offset = 8;
  //   text_layer_set_text(s_day_layer, "14");
  //   layer_mark_dirty(s_bars_layer);
  // }else{
  //   window_stack_remove(s_dots_history_window, false);
  //   psleep(100); // wait a bit for the OS to reset the window pointers
  //   display_main_dash();
  // }
  window_stack_remove(s_acti_history_window, false);
  psleep(100); // wait a bit for the OS to reset the window pointers
  display_main_dash();
}

static void click_config_provider(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}



static void barss_layer_update_proc(struct Layer *layer, GContext *ctx){

  graphics_context_set_stroke_color(ctx, GColorWhite);
  #ifdef PBL_SDK_3
    graphics_context_set_stroke_width(ctx, 3);
  #endif

  graphics_draw_line(ctx, GPoint(axis_x,mid_y + step_h*2),GPoint(axis_x,mid_y - step_h*2));
  graphics_draw_line(ctx, GPoint(axis_x,mid_y - step_h*2),GPoint(axis_x-4,mid_y - step_h*2));
  graphics_draw_line(ctx, GPoint(axis_x,mid_y - step_h*1),GPoint(axis_x-4,mid_y - step_h*1));
  graphics_draw_line(ctx, GPoint(bar_w/2 +5,mid_y - step_h*0),GPoint(axis_x,mid_y - step_h*0));
  graphics_draw_line(ctx, GPoint(axis_x,mid_y + step_h*1),GPoint(axis_x-4,mid_y + step_h*1));
  graphics_draw_line(ctx, GPoint(axis_x,mid_y + step_h*2),GPoint(axis_x-4,mid_y + step_h*2));

  for(int8_t i = 0; i < num_bars; i++){
    int8_t state = get_pinteract_state().pi_11[i+day_offset];

    if( state >= 0){
      int16_t circle_y = mid_y + (state -2)*step_h;
      graphics_context_set_fill_color(ctx, GColorWhite);

      graphics_draw_line(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y),
        GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,mid_y));
      graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y), bar_w/2-2);

    }
  }
}

static void window_load(Window* window){
  // NOTE!!, we are essentially trying to immitate Misfit and jawbone,
  // so white text on black background
  Layer* window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);
  // ADD THE MOVING GRAPHICS MUST BE AT TOP OF LAYER STACK
  s_bars_layer = layer_create(window_bounds);
  layer_set_update_proc(s_bars_layer, barss_layer_update_proc);
  //   layer_set_background_color(s_points_layer, GColorBlack);
  layer_add_child(window_layer, s_bars_layer);
  layer_mark_dirty(s_bars_layer);

  // add title layer
  s_title_layer = text_layer_create(GRect(20,-2,window_bounds.size.w-40,25));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_title_layer, GColorBlack);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  if(history_acti_type == 1){
    text_layer_set_text(s_title_layer, "Steps");
  }else if(history_acti_type == 2){
    text_layer_set_text(s_title_layer, "Calories");
  }
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  // add some logic here to add the Sunday marker, but depends whether longer
  // or shorter

  s_day_layer = text_layer_create(GRect(3,-2,22,25));
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  text_layer_set_font(s_day_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_day_layer, GColorBlack);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_text(s_day_layer, "7");
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));





  time_t init_t = time(NULL);
  // run the tick handler once to update everything
  struct tm *tick_time = localtime( &init_t );
  num_bars = 7;
  axis_x = window_bounds.size.w -3;
  bar_w = ((window_bounds.size.w*10/12 )/num_bars);

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
      window_bounds.size.h-17,17,15) );
    text_layer_set_text_alignment(s_wday_layers[i], GTextAlignmentCenter);
    text_layer_set_font(s_wday_layers[i],fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_background_color(s_wday_layers[i], GColorBlack);
    text_layer_set_text_color(s_wday_layers[i], GColorWhite);
    text_layer_set_text(s_wday_layers[i], weekday[ (tick_time->tm_wday -i -1+7)%7 ]);
    layer_add_child(window_layer, text_layer_get_layer(s_wday_layers[i]));
  }

  // pinteract 11
  mid_y = window_bounds.size.h/2 + 5;
  step_h = (window_bounds.size.h*9/12)/5;

  // pinteract 14 -> sleep rating

  // ONLY USE this for 5 scale ratings.
  //  use bars for sleep, steps, and kcals
}


static void window_unload(Window* window){
  // we sure to destroy the moving graphics!!!

  // Destroy all the graphics
  layer_destroy(s_bars_layer);
  text_layer_destroy(s_title_layer);
  for(int16_t i = 0; i < 7; i++){
    text_layer_destroy(s_wday_layers[i]);
  }
  psleep(100);
}



void display_acti_bars_history(int16_t acti_type){
  history_acti_type = acti_type;
  day_offset = 1;
  // WHENEVER we access the main dash, we just want to clean everything up
  window_stack_pop_all(false);
  psleep(100); // NEEDED so OS clears prev window and frees RAM

  // get the points goal from the config file
  s_acti_history_window = window_create();

  window_set_window_handlers(s_acti_history_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });

    // cause this function is not defined in basalt, cause it doesn't have status bar
  #ifdef PBL_SDK_2
    window_set_fullscreen(s_acti_history_window,true);
  #endif

  window_set_click_config_provider(s_acti_history_window,
                                   (ClickConfigProvider) click_config_provider);

  window_stack_push(s_acti_history_window,false);
}
