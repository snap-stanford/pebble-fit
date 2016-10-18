// this is a file that simply displays the last 10 days of mood data

// be sure to write into the background worker to move the day over at midnight
#include "display_func.h"



static Window* s_dots_history_window;
static Layer* s_dots_layer;
static TextLayer* s_title_layer;
static TextLayer* s_dates_layer;
static TextLayer* s_day_layer;
static TextLayer* s_axis_y_label_layer;
static TextLayer* s_wday_layers[7];


static int16_t num_bars;
static int16_t bar_w;
static int16_t mid_y;
static int16_t step_h;
// create the axis
static int16_t axis_x;

static int32_t max_steps;
static int32_t steps_goal_line_y;
static int32_t max_kcal;
static int32_t max_sleep_min;

// static int32_t min_kcal;


// text layer for title :  "mood history"
static int16_t history_pi_code;
static int16_t day_offset;


static void set_title_dates_y_axis_label(){
  switch(history_pi_code){
    case 1:
      text_layer_set_text(s_title_layer, "Steps");
      text_layer_set_text(s_axis_y_label_layer, "Goal");
      max_steps = 0;
      // min_kcal = 100000;
      for(int16_t i = 0; i < num_bars; i++ ){
        uint16_t comp_steps = get_daily_acti().steps[i+day_offset];
        int32_t steps = comp_steps*comp_steps;
        APP_LOG(APP_LOG_LEVEL_ERROR, "steps1: %d",(int) steps);
        if(steps > max_steps){max_steps = steps;}
      }
      // max_steps = (max_steps/100)*100;  // round to nearest hundred
      APP_LOG(APP_LOG_LEVEL_ERROR, "max_steps: %d",(int) max_steps);
      if(((max_steps*5)/6) < get_config_general().pts_goal ){
        APP_LOG(APP_LOG_LEVEL_ERROR, "<");
        steps_goal_line_y = mid_y - step_h*1;
      }else{
        APP_LOG(APP_LOG_LEVEL_ERROR, ">");
        steps_goal_line_y = mid_y + 2*step_h - ((4*step_h*get_config_general().pts_goal)/max_steps);
      }
      APP_LOG(APP_LOG_LEVEL_ERROR, "steps_goal_line_y: %d",(int) steps_goal_line_y);
      layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-52,steps_goal_line_y-23,50,21));
      break;
    #ifdef PBL_COLOR
    case 2:
      text_layer_set_text(s_title_layer, "Activity Calories");
      max_kcal = 0;
      // min_kcal = 100000;
      for(int16_t i = 0; i < num_bars; i++ ){
        int8_t comp_kcal = get_daily_acti().kcal[i+day_offset];
        int32_t kcal = comp_kcal*comp_kcal;
        if(kcal > max_kcal){max_kcal = kcal;}
        // if(kcal < min_kcal){min_kcal = kcal;}
      }
      max_kcal = ((max_kcal/100) +1)*100;
      // add max kcal layer
      static char kcal_buf[6];
      snprintf(kcal_buf,sizeof(kcal_buf),"%d",(int)max_kcal);
      text_layer_set_text(s_axis_y_label_layer, kcal_buf);
      layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-52,20,50,21));

      break;

    case 11:
      text_layer_set_text(s_title_layer, "Mood");
      text_layer_set_text(s_axis_y_label_layer, "");
      layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-52,mid_y-23,50,21));
      break;

    case 140:

      max_sleep_min = 0;
      int16_t avg_sleep_min = 0;
      int16_t num_non_zero_sleep_min = 0;
      // min_kcal = 100000;
      for(int16_t i = 0; i < num_bars; i++ ){
        int16_t sleep_min = get_pinteract_state().pi_140[i+day_offset];
        if(sleep_min > 0){
          avg_sleep_min += sleep_min;
          num_non_zero_sleep_min += 1;
        }
        if(sleep_min > max_sleep_min){max_sleep_min = sleep_min;}
        // if(kcal < min_kcal){min_kcal = kcal;}
      }
      max_sleep_min = (max_sleep_min/60 +1)*60;
      avg_sleep_min /= num_non_zero_sleep_min;

      static char sleep_title_buf[20];
      snprintf(sleep_title_buf,sizeof(sleep_title_buf),"Average Sleep %d:%.2d",
        (int)(avg_sleep_min/60), (int)(avg_sleep_min%60)  );
      text_layer_set_text(s_title_layer, sleep_title_buf);

      static char sleep_min_buf[6];
      snprintf(sleep_min_buf,sizeof(sleep_min_buf),"%d hrs",(int)(max_sleep_min/60));
      layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-52,20,50,21));
      text_layer_set_text(s_axis_y_label_layer, sleep_min_buf);
      break;

    case 141:
      text_layer_set_text(s_title_layer, "Sleep Quality");
      text_layer_set_text(s_axis_y_label_layer, "");
      layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-52,mid_y-23,50,21));

      break;
    #endif
  }
  layer_mark_dirty((Layer *)s_axis_y_label_layer);
  layer_mark_dirty((Layer *)s_title_layer);

}

static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  day_offset = 1;
  layer_mark_dirty(s_dots_layer);
  text_layer_set_text(s_day_layer, "7");
  // DONT CLOSE OUT THE DASH WINDOW!!
  // display_detail_dash();
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context){
  // nothing yet
  // #ifdef PBL_COLOR
  //   display_pinteract_circle_hist(history_pi_code);
  // #endif
  // #ifdef PBL_COLOR
  #ifdef PBL_COLOR
  switch(history_pi_code){
    case 1:
      history_pi_code = 2;
      break;
    case 2:
      history_pi_code = 140;
      break;
    case 11:
      window_stack_remove(s_dots_history_window, false);
      display_main_dash();
      return;
    case 140:
      history_pi_code = 141;
      break;
    case 141:
      history_pi_code = 11;
      break;
  }
  #else
  switch(history_pi_code){
    case 1:
      window_stack_remove(s_dots_history_window, false);
      display_main_dash();
      return;
  }
  #endif
  set_title_dates_y_axis_label();
  layer_mark_dirty(s_dots_layer);

}

static void down_click_handler(ClickRecognizerRef recognizer, void *context){
  // psleep(100); // wait a bit for the OS to reset the window pointers
  #ifdef PBL_COLOR
  switch(history_pi_code){
    case 1:
      window_stack_remove(s_dots_history_window, false);
      display_main_dash();
      return;
    case 2:
      history_pi_code = 1;
      break;
    case 11:
      history_pi_code = 141 ;
      break;
    case 140:
      history_pi_code = 2;
      break;
    case 141:
      history_pi_code = 140;
      break;
  }
  #else
  switch(history_pi_code){
    case 1:
      window_stack_remove(s_dots_history_window, false);
      display_main_dash();
      return;
  }
  #endif
  set_title_dates_y_axis_label();
  layer_mark_dirty(s_dots_layer);

}


static void back_click_handler(ClickRecognizerRef recognizer, void *context){
  // if(day_offset == 1){
  //   day_offset = 8;
  //   text_layer_set_text(s_day_layer, "14");
  //   layer_mark_dirty(s_dots_layer);
  // }else{
  //   window_stack_remove(s_dots_history_window, false);
  //   psleep(100); // wait a bit for the OS to reset the window pointers
  //   display_main_dash();
  // }
  window_stack_remove(s_dots_history_window, false);
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


static void dots_layer_update_proc(struct Layer *layer, GContext *ctx){

  graphics_context_set_stroke_color(ctx, GColorWhite);
  #ifdef PBL_SDK_3
    graphics_context_set_stroke_width(ctx, 3);
  #endif
  GRect layer_bounds = layer_get_bounds(layer);
  int16_t circle_y = 0;
  int16_t circle_y_0 = 0;
  int16_t circle_y_1 = 0;
  int16_t y_displace = 0;

  switch(history_pi_code){
    case 1:
      // draw the goal line
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,steps_goal_line_y),
        GPoint(axis_x,steps_goal_line_y));
      // draw the bottom bar
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,mid_y + step_h*2),
        GPoint((num_bars-1)*bar_w + bar_w/2 +5,mid_y + step_h*2));
      for(int16_t i = 0; i < num_bars; i++){
        uint16_t comp_steps = get_daily_acti().steps[i+day_offset];
        int32_t steps = comp_steps*comp_steps;

        APP_LOG(APP_LOG_LEVEL_ERROR, "steps: %d",(int) steps);

        if(((max_steps*5)/6) < get_config_general().pts_goal ){
          circle_y = mid_y + 2*step_h - ((3*step_h*steps)/get_config_general().pts_goal );
        }else{
          circle_y = mid_y + 2*step_h - ((4*step_h*steps)/max_steps);
        }

        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_draw_line(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y),
          GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,mid_y + 2*step_h));
        graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y), bar_w/2-4);

      }
      break;

    #ifdef PBL_COLOR
    case 2:
      // bottom of graph
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,mid_y + step_h*2),
        GPoint((num_bars-1)*bar_w + bar_w/2 +5,mid_y + step_h*2));
      // line level with the max number
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,35),
        GPoint(axis_x - 40,35));
      // draw lines one at a time
      y_displace = 5;

      for(int16_t i = 0; i < (num_bars-1); i++){

        uint16_t comp_kcal_0 = get_daily_acti().kcal[i+day_offset];
        int32_t kcal_0 = comp_kcal_0*comp_kcal_0;
        circle_y_0 = mid_y +2*step_h - ((4*step_h*kcal_0)/max_kcal) ;
        circle_y_0 = ((circle_y_0+y_displace)<= (mid_y + step_h*2))? (circle_y_0+y_displace) : circle_y_0;

        uint16_t comp_kcal_1 = get_daily_acti().kcal[i+day_offset+1];
        int32_t kcal_1 = comp_kcal_1*comp_kcal_1;
        circle_y_1 = mid_y +2*step_h - ((4*step_h*kcal_1)/max_kcal);
        circle_y_1 = ((circle_y_1+y_displace)<= (mid_y + step_h*2))? (circle_y_1+y_displace) : circle_y_1;

        graphics_context_set_fill_color(ctx, GColorWhite);
        graphics_draw_line(ctx, GPoint((num_bars-2-i)*bar_w + bar_w/2 +5 ,circle_y_1),
          GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y_0));
        // graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y_1), bar_w/2-4);
        graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y_0), bar_w/2-4);
      }
      graphics_fill_circle(ctx, GPoint((0)*bar_w + bar_w/2 +5 ,circle_y_1), bar_w/2-4);
      break;

    case 11:
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,mid_y - step_h*0),
        GPoint((num_bars-1)*bar_w + bar_w/2 +5,mid_y - step_h*0));
      for(int16_t i = 0; i < num_bars; i++){
        int8_t state = get_pinteract_state().pi_11[i+day_offset];
        if( (state >= 0)){
          int16_t circle_y = mid_y + (state -2)*step_h;

          graphics_context_set_fill_color(ctx, GColorWhite);
          graphics_draw_line(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y),
            GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,mid_y));

          graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y), bar_w/2-4);
        }
      }
      break;

    case 140:
      // line at the bottom of the graph
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,mid_y + step_h*2),
        GPoint((num_bars-1)*bar_w + bar_w/2 +5,mid_y + step_h*2));
      // line level with the max value
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,35),
        GPoint(axis_x - 40,35));

      // draw lines one at a time
      y_displace = 5;

      for(int16_t i = 0; i < (num_bars-1); i++){

        int32_t sleep_min_0 = get_pinteract_state().pi_140[i+day_offset];
        circle_y_0 = mid_y +2*step_h - ((4*step_h*sleep_min_0)/max_sleep_min) ;
        circle_y_0 = ((circle_y_0+y_displace)<= (mid_y + step_h*2))? (circle_y_0+y_displace) : circle_y_0;

        int32_t sleep_min_1 = get_pinteract_state().pi_140[i+day_offset+1];
        circle_y_1 = mid_y +2*step_h - ((4*step_h*sleep_min_1)/max_sleep_min);
        circle_y_1 = ((circle_y_1+y_displace)<= (mid_y + step_h*2))? (circle_y_1+y_displace) : circle_y_1;


        graphics_context_set_fill_color(ctx, GColorWhite);
        if( get_pinteract_state().pi_140[i+day_offset] >= 0
          && get_pinteract_state().pi_140[i+day_offset+1] >= 0){
          graphics_draw_line(ctx, GPoint((num_bars-2-i)*bar_w + bar_w/2 +5 ,circle_y_1),
            GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y_0));
        }
        if( get_pinteract_state().pi_140[i+day_offset] >= 0 ){
        // graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y_1), bar_w/2-4);
          graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y_0), bar_w/2-4);
        }
      }
      // add the last dot at the 7th day
      if(get_pinteract_state().pi_140[(num_bars-1)+day_offset] >= 0){
        graphics_fill_circle(ctx, GPoint((0)*bar_w + bar_w/2 +5 ,circle_y_1), bar_w/2-4);
      }

      break;
    case 141:
      graphics_draw_line(ctx, GPoint(bar_w/2 +5,mid_y - step_h*0),
        GPoint((num_bars-1)*bar_w + bar_w/2 +5,mid_y - step_h*0));
      for(int16_t i = 0; i < num_bars; i++){
        int8_t state = get_pinteract_state().pi_141[i+day_offset];
        if( (state >= 0)){
          int16_t circle_y = mid_y + (state -2)*step_h;
          graphics_context_set_fill_color(ctx, GColorWhite);
          graphics_draw_line(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y),
            GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,mid_y));
          graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +5 ,circle_y), bar_w/2-4);
        }
      }
      break;
    #endif
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
  // pinteract 11 & 1
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
  APP_LOG(APP_LOG_LEVEL_ERROR, "window unload : heap size: used %d , free %d",
      heap_bytes_used(), heap_bytes_free());
  // psleep(100);
}

void display_pinteract_dots_history(int16_t history_pi_code_in){
  APP_LOG(APP_LOG_LEVEL_ERROR, "window start : heap size: used %d , free %d",
      heap_bytes_used(), heap_bytes_free());
  history_pi_code = history_pi_code_in;
  day_offset = 1;
  // WHENEVER we access the main dash, we just want to clean everything up
  // window_stack_pop_all(false);
  // psleep(100); // NEEDED so OS clears prev window and frees RAM

  // get the points goal from the config file
  s_dots_history_window = window_create();

  window_set_window_handlers(s_dots_history_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });

    // cause this function is not defined in basalt, cause it doesn't have status bar
  #ifdef PBL_SDK_2
    window_set_fullscreen(s_dots_history_window,true);
  #endif

  window_set_click_config_provider(s_dots_history_window,
                                   (ClickConfigProvider) click_config_provider);

  window_stack_push(s_dots_history_window,false);
}


// if(state != 2){
//   int16_t bar_h = (state >= 0) ? ((2 -state)*step_h) : 0;
//   int16_t bar_y = (bar_h < 0) ? (mid_y+1) : (mid_y - bar_h);
//
//   // #ifdef PBL_COLOR
//   //   switch(state){
//   //     case 0 :
//   //       graphics_context_set_fill_color(ctx, GColorOrange);
//   //       break;
//   //     case 1 :
//   //       graphics_context_set_fill_color(ctx, GColorBrass);
//   //       break;
//   //     case 3 :
//   //       graphics_context_set_fill_color(ctx, GColorCadetBlue);
//   //       break;
//   //     case 4 :
//   //       graphics_context_set_fill_color(ctx, GColorBlueMoon);
//   //       break;
//   //   }
//   // #else
//     graphics_context_set_fill_color(ctx, GColorWhite);
//   // #endif
//
//   graphics_fill_rect(ctx, GRect((num_bars-1-i)*bar_w +5, bar_y,
//     bar_w - 1, abs(bar_h)),2, GCornersAll);
// }else{
//   // #ifdef PBL_COLOR
//   //   graphics_context_set_fill_color(ctx, GColorIslamicGreen);
//   // #else
//     graphics_context_set_fill_color(ctx, GColorWhite);
//   // #endif
//   graphics_fill_circle(ctx, GPoint((num_bars-1-i)*bar_w + bar_w/2 +4 , mid_y), bar_w/2);
// }
//





// static void update_to_pinteract_layer(){
//   switch(history_pi_code){
//     case 1:
//       layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-41,mid_y-16,40,15));
//       text_layer_set_text(s_title_layer, "Steps");
//       text_layer_set_text(s_axis_y_label_layer, "Goal");
//       layer_mark_dirty((Layer *)s_axis_y_label_layer);
//       layer_mark_dirty(s_dots_layer);
//       break;
//     case 2:
//       max_kcal = 0;
//       for(int16_t i = 0; i < num_bars; i++ ){
//         int8_t comp_kcal = get_daily_acti().kcal[i+day_offset];
//         int32_t kcal = comp_kcal*comp_kcal;
//         if(kcal > max_kcal){max_kcal = kcal;}
//       }
//       // round to near
//       max_kcal = (max_kcal/100)*100;
//       text_layer_set_text(s_title_layer, "Calories");
//       layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-40,mid_y-16 -2*step_h,40,15));
//       static char max_kcal_buf[7];
//       snprintf(max_kcal_buf,sizeof(max_kcal_buf),"%d", (int)max_kcal);
//       text_layer_set_text(s_axis_y_label_layer, max_kcal_buf);
//       layer_mark_dirty((Layer *)s_axis_y_label_layer);
//       layer_mark_dirty(s_dots_layer);
//       break;
//     case 11:
//       text_layer_set_text(s_title_layer, "Mood");
//       text_layer_set_text(s_axis_y_label_layer, "Norm");
//       layer_mark_dirty(s_dots_layer);
//
//       break;
//     case 140:
//       break;
//     case 141:
//       break;
//   }
//
// }
//
// static void select_click_handler(ClickRecognizerRef recognizer, void *context){
//   day_offset = 1;
//   layer_mark_dirty(s_dots_layer);
//   text_layer_set_text(s_day_layer, "7");
//   // DONT CLOSE OUT THE DASH WINDOW!!
//   // display_detail_dash();
// }
//
//
// static void up_click_handler(ClickRecognizerRef recognizer, void *context){
//   // nothing yet
//   // #ifdef PBL_COLOR
//   //   display_pinteract_circle_hist(history_pi_code);
//   // #endif
//   // #ifdef PBL_COLOR
//   if(history_pi_code == 1){
//     history_pi_code = 2;
//     update_to_pinteract_layer();
//     // max_kcal = 0;
//     // for(int16_t i = 0; i < num_bars; i++ ){
//     //   int8_t comp_kcal = get_daily_acti().kcal[i+day_offset];
//     //   int32_t kcal = comp_kcal*comp_kcal;
//     //   if(kcal > max_kcal){max_kcal = kcal;}
//     // }
//     // // round to near
//     // max_kcal = (max_kcal/100)*100;
//     // text_layer_set_text(s_title_layer, "Calories");
//     // layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-40,mid_y-16 -2*step_h,40,15));
//     // static char max_kcal_buf[7];
//     // snprintf(max_kcal_buf,sizeof(max_kcal_buf),"%d", (int)max_kcal);
//     // text_layer_set_text(s_axis_y_label_layer, max_kcal_buf);
//     // layer_mark_dirty((Layer *)s_axis_y_label_layer);
//     // layer_mark_dirty(s_dots_layer);
//
//
//   }else if(history_pi_code ==2){
//
//   }else if(history_pi_code ==11){
//     // psleep(100); // wait a bit for the OS to reset the window pointers
//     // display_acti_bars_history(2);
//     history_pi_code = 1;
//     update_to_pinteract_layer();
//     // text_layer_set_text(s_title_layer, "Steps");
//     // text_layer_set_text(s_axis_y_label_layer, "Goal");
//     // layer_mark_dirty(s_dots_layer);
//
//
//   }
// }
//
// static void down_click_handler(ClickRecognizerRef recognizer, void *context){
//   // psleep(100); // wait a bit for the OS to reset the window pointers
//
//   if(history_pi_code == 1){
//     // display_acti_bars_history(2);
//     history_pi_code = 11;
//     update_to_pinteract_layer();
//     // layer_mark_dirty(s_dots_layer);
//     // text_layer_set_text(s_title_layer, "Mood");
//     // text_layer_set_text(s_axis_y_label_layer, "Norm");
//   }else if(history_pi_code ==2){
//     history_pi_code = 1;
//     update_to_pinteract_layer();
//     // layer_set_frame((Layer *)s_axis_y_label_layer, GRect(axis_x-41,mid_y-16,40,15));
//     // text_layer_set_text(s_title_layer, "Steps");
//     // text_layer_set_text(s_axis_y_label_layer, "Goal");
//     // layer_mark_dirty((Layer *)s_axis_y_label_layer);
//     // layer_mark_dirty(s_dots_layer);
//
//   }else if(history_pi_code == 11){
//     window_stack_remove(s_dots_history_window, false);
//     display_main_dash();
//   }
// }


// char* month[12];
// month[0]="Jan";
// month[1]="Feb";
// month[2]="Mar";
// month[3]="Apr";
// month[4]="May";
// month[5]="Jun";
// month[6]="Jul";
// month[7]="Aug";
// month[8]="Sep";
// month[9]="Oct";
// month[10]="Nov";
// month[11]="Dec";
// int16_t cur_day = tick_time->tm_mday;
// int16_t cur_mon = tick_time->tm_mon;
// static char dates_buf[14];
// snprintf(dates_buf,sizeof(dates_buf),"%s %d",month[cur_mon],cur_day);
// s_dates_layer = text_layer_create(GRect( window_bounds.size.w -45 , 2,40,21) );
// text_layer_set_text_alignment(s_dates_layer, GTextAlignmentRight);
// text_layer_set_font(s_dates_layer,fonts_get_system_font(FONT_KEY_GOTHIC_18));
// text_layer_set_background_color(s_dates_layer, GColorBlack);
// text_layer_set_text_color(s_dates_layer, GColorWhite);
// text_layer_set_text(s_dates_layer,dates_buf);
// layer_add_child(window_layer, text_layer_get_layer(s_dates_layer));



// graphics_draw_line(ctx, GPoint(axis_x,mid_y + step_h*2),GPoint(axis_x,mid_y - step_h*2));
// graphics_draw_line(ctx, GPoint(axis_x,mid_y - step_h*2),GPoint(axis_x-5,mid_y - step_h*2));
// graphics_draw_line(ctx, GPoint(axis_x,mid_y - step_h*1),GPoint(axis_x-5,mid_y - step_h*1));
// graphics_draw_line(ctx, GPoint(axis_x,mid_y + step_h*1),GPoint(axis_x-5,mid_y + step_h*1));
// graphics_draw_line(ctx, GPoint(axis_x,mid_y + step_h*2),GPoint(axis_x-5,mid_y + step_h*2));
