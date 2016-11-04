#include "wakeup_window.h"

static Window *s_window;
static TextLayer *s_main_text_layer, *s_text_above_layer, *s_text_below_layer;

static int s_latest_step;
static char s_start[12], s_end[12];
static char s_text_above_buf[40];
static char s_text_below_buf[40];
  

//static TextLayer *s_debug1_layer;
static int s_rest_mins;

void wakeup_window_update_steps(int steps, char *start, char *end, int entries) {
    s_latest_step = steps;
    strncpy(s_start, start, sizeof(s_start));
    strncpy(s_end, end, sizeof(s_end));
    s_rest_mins = entries;
}

/* Set standard attributes on new text layer in this window. */
static TextLayer* make_text_layer(GRect bounds, GFont font, GTextAlignment align) {
  TextLayer *this = text_layer_create(bounds);
  text_layer_set_background_color(this, GColorClear);
  text_layer_set_text_color(this, GColorBlack);
  text_layer_set_text_alignment(this, align);
  text_layer_set_font(this, font);
  return this;
}

/* Set steps text according to number of steps taken. */
//static void update_steps_text(char * s_buffer, int s_buf_len, int steps, char * prefix, TextLayer * layer) {
//  int thousands = steps / 1000;
//  if (thousands > 0) {
//    int hundreds = steps / 100;
//    snprintf(s_buffer, s_buf_len, "%s%d.%dk", prefix, thousands, hundreds);
//  } else {
//    snprintf(s_buffer, s_buf_len, "%s%d", prefix, steps);
//  }
//
//  text_layer_set_text(layer, s_buffer);
//}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  int padding = PBL_IF_ROUND_ELSE(10, 0);
  float center = bounds.size.h / 2;
  int text_above_height = 50;
  int main_text_height = PBL_IF_ROUND_ELSE(60, 70);
  int text_below_height = 40;

  // The text layer that is above the main text layer
  GEdgeInsets text_above_insets = {.top = 10, .bottom = 10 + text_above_height, 
    .right = 10, .left = 10};
  s_text_above_layer = make_text_layer(
    grect_inset(bounds, text_above_insets),
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter);
  APP_LOG(APP_LOG_LEVEL_INFO, "Steps: %d", s_latest_step);
  snprintf(s_text_above_buf, sizeof(s_text_above_buf), 
           "Steps: %d\n%s-%s", s_latest_step, s_start, s_end);
  text_layer_set_text(s_text_above_layer, s_text_above_buf);

  // The main text layer that resides in the center of the screen
  s_main_text_layer = make_text_layer(
    GRect(0, center - main_text_height/2  - padding, bounds.size.w, main_text_height), 
    fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK), GTextAlignmentCenter);
  if (s_latest_step > enamel_get_step_threshold()) {
    //text_layer_set_text(s_main_text_layer, "Keep\nup");
    text_layer_set_text(s_main_text_layer, "Let's\nMove");
  } else {
    text_layer_set_text(s_main_text_layer, "Let's\nMove");
    // TODO: maybe move this vibration to somewhere else
    if (enamel_get_vibrate()) vibes_short_pulse();
  }

  // The text layer that is below the main text layer
  GEdgeInsets text_below_insets = {.top = center+main_text_height/2, .right = 10, .left = 10};
  s_text_below_layer = make_text_layer(
    grect_inset(bounds, text_below_insets),
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter);
  snprintf(s_text_below_buf, sizeof(s_text_below_buf), 
           "Been resting for the last %dmins", s_rest_mins);
  text_layer_set_text(s_text_below_layer, s_text_below_buf);

  // Add text layers to the window
  layer_add_child(root_layer, text_layer_get_layer(s_main_text_layer));
  layer_add_child(root_layer, text_layer_get_layer(s_text_above_layer));
  layer_add_child(root_layer, text_layer_get_layer(s_text_below_layer));
}

static void window_unload(Window *window) {
  if (s_main_text_layer) {
    text_layer_destroy(s_main_text_layer);
    s_main_text_layer = NULL;
  }
  if (s_text_above_layer) {
    text_layer_destroy(s_text_above_layer);
    s_text_above_layer = NULL;
  }
  if (s_text_below_layer) {
    text_layer_destroy(s_text_below_layer);
    s_text_below_layer = NULL;
  }
  //if (s_debug1_layer) {
  //  text_layer_destroy(s_debug1_layer);
  //  s_debug1_layer = NULL;
  //}

  window_destroy(s_window);
  s_window = NULL;
}

/* Create a window and push to the window stack. */
void wakeup_window_push() {
  if (!s_window) {
    s_window = window_create();

    window_set_window_handlers(s_window, (WindowHandlers) {
      .load  = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}

/* Pop window from the window stack. */
void wakeup_window_remove() {
  window_stack_remove(s_window, false);
}
