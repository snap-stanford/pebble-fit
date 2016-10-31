#include "wakeup_window.h"

static Window *s_window;
static TextLayer *s_title_layer, *s_subtitle_layer, *s_foot_layer;
static int s_latest_step;
static char s_start[12], s_end[12];

static TextLayer *s_debug1_layer;
static int s_debug_entry_count;

void wakeup_window_update_steps(int steps, char *start, char *end, int entries) {
    s_latest_step = steps;
    strncpy(s_start, start, sizeof(s_start));
    strncpy(s_end, end, sizeof(s_end));

    s_debug_entry_count = entries;
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
static void update_steps_text(char * s_buffer, int s_buf_len, int steps, char * prefix, TextLayer * layer) {
  int thousands = steps / 1000;
  if (thousands > 0) {
    int hundreds = steps / 100;
    snprintf(s_buffer, s_buf_len, "%s%d.%dk", prefix, thousands, hundreds);
  } else {
    snprintf(s_buffer, s_buf_len, "%s%d", prefix, steps);
  }

  text_layer_set_text(layer, s_buffer);
}

static void window_load(Window *window) {
  if (s_latest_step > enamel_get_step_threshold()) {
    return; // If step goal is met TODO: refine
  }
  Layer *root_layer = window_get_root_layer(window);

  GRect bounds = layer_get_bounds(root_layer);

  // Create text layers, and set their texts
  int padding = 15;
  float center = bounds.size.h / 2 - padding;
  
  int title_height = 60;
  s_title_layer = make_text_layer(GRect(0, center - title_height, bounds.size.w, title_height), fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK), GTextAlignmentCenter);
  if (s_latest_step > enamel_get_step_threshold()) {
    text_layer_set_text(s_title_layer, "Keep up");
  } else {
    text_layer_set_text(s_title_layer, "Let's Move");
    if (enamel_get_vibrate()) vibes_short_pulse();
  }

  int subtitle_height = 25;
  s_subtitle_layer = make_text_layer(GRect(0, center, bounds.size.w, subtitle_height), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter);
  static char step_buf[16];
  APP_LOG(APP_LOG_LEVEL_INFO, "Steps: %d", s_latest_step);
  snprintf(step_buf, sizeof(step_buf), "Steps: %d", s_latest_step);
  text_layer_set_text(s_subtitle_layer, step_buf);

  int foot_height = 22;
  s_foot_layer = make_text_layer(GRect(0, center + subtitle_height, bounds.size.w, foot_height), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter);
  static char time_buf[32];
  snprintf(time_buf, sizeof(time_buf), "%s-%s", s_start, s_end);
  text_layer_set_text(s_foot_layer, time_buf);

  int debug1_height = 40;
  s_debug1_layer = make_text_layer(GRect(0, center + subtitle_height + foot_height, bounds.size.w, debug1_height), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter);
  static char debug1_buf[32];
  snprintf(debug1_buf, sizeof(debug1_buf), "Been resting for the last %dmins", s_debug_entry_count);
  text_layer_set_text(s_debug1_layer, debug1_buf);

  // Add text layer to the window
  layer_add_child(root_layer, text_layer_get_layer(s_title_layer));
  layer_add_child(root_layer, text_layer_get_layer(s_subtitle_layer));
  layer_add_child(root_layer, text_layer_get_layer(s_foot_layer));
  layer_add_child(root_layer, text_layer_get_layer(s_debug1_layer));
}

static void window_unload(Window *window) {
  if (s_title_layer) {
    text_layer_destroy(s_title_layer);
    s_title_layer = NULL;
  }
  if (s_subtitle_layer) {
    text_layer_destroy(s_subtitle_layer);
    s_subtitle_layer = NULL;
  }
  if (s_foot_layer) {
    text_layer_destroy(s_foot_layer);
    s_foot_layer = NULL;
  }
  if (s_debug1_layer) {
    text_layer_destroy(s_debug1_layer);
    s_debug1_layer = NULL;
  }

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
