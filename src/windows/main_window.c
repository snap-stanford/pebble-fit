#include "main_window.h"

static Window *s_window;
static TextLayer *title_layer;
static TextLayer *subtitle_layer;
static TextLayer *foot_layer;
static Layer *s_progress_layer;

static int s_step_count = 0, s_step_goal = 0, s_step_average = 0;

/* Set standard attributes on new text layer. */
static TextLayer* make_text_layer(GRect bounds, GFont font, GTextAlignment align) {
  TextLayer *this = text_layer_create(bounds);
  text_layer_set_background_color(this, GColorClear);
  text_layer_set_text_color(this, GColorWhite);
  text_layer_set_text_alignment(this, align);
  text_layer_set_font(this, font);
  return this;
}

/* calculate rect which will enclose the progress circle. */
static GRect calculate_rect(Layer *layer, uint8_t arc_id) {
  uint8_t padding = 4;
  return grect_inset(layer_get_bounds(layer), GEdgeInsets(padding*(arc_id)));
}

/* Repaint the context with new (static var) values. */
static void progress_layer_update_proc(Layer *layer, GContext *ctx) {
  int thickness = 20;
  graphics_context_set_fill_color(ctx, GColorOrange);
  graphics_fill_radial(ctx, calculate_rect(layer, 2), GOvalScaleModeFitCircle,
    thickness / 4, 0, TRIG_MAX_ANGLE);


  graphics_context_set_fill_color(ctx, 
    s_step_count >= s_step_average ? GColorGreen : GColorIcterine);

  graphics_fill_radial(ctx, calculate_rect(layer, 1), GOvalScaleModeFitCircle, thickness,
    DEG_TO_TRIGANGLE(0),
    DEG_TO_TRIGANGLE(360 * (s_step_count * 1.0 / s_step_goal)));
}

/* Setup layers with initial text. */
static void window_load(Window * window) {
  // Get the bounds of the root layer
  Layer *root_layer  = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  GRect bounds = layer_get_bounds(root_layer);

  s_progress_layer = layer_create(bounds);
  layer_set_update_proc(s_progress_layer, progress_layer_update_proc);

  // Create text layers, and set their texts
  float center = bounds.size.h / 2;
  int height = 30;
  title_layer = make_text_layer(GRect(0, center - height, bounds.size.w, height), fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK), GTextAlignmentCenter);
  text_layer_set_text(title_layer, "");

  subtitle_layer = make_text_layer(GRect(0, center, bounds.size.w, center + height), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter);
  text_layer_set_text(subtitle_layer, "");

  int foot_height = 22;
  foot_layer = make_text_layer(GRect(0, bounds.size.h - foot_height, bounds.size.w, foot_height), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentCenter);
  text_layer_set_text(foot_layer, "Keep going!");

  // Add text layer to the window
  layer_add_child(root_layer, s_progress_layer);
  layer_add_child(root_layer, text_layer_get_layer(title_layer));
  layer_add_child(root_layer, text_layer_get_layer(subtitle_layer));
  layer_add_child(root_layer, text_layer_get_layer(foot_layer));
}

/* Destroy elements before destroying window. */
static void window_unload(Window *window) {
  text_layer_destroy(title_layer);
  text_layer_destroy(subtitle_layer);
  text_layer_destroy(foot_layer);
  layer_destroy(s_progress_layer);
  window_destroy(s_window);
}

/* Set steps text according to number of steps taken. */
static void update_steps_text(char * s_buffer, int s_buf_len, int steps, char * prefix, TextLayer * layer) {
  // prepare format
  int thousands = steps / 1000;
  if(thousands > 0) {
    int hundreds = steps / 100;
    snprintf(s_buffer, s_buf_len,
      "%s%d.%dk", prefix, thousands, hundreds);
  } else {
    snprintf(s_buffer, s_buf_len,
      "%s%d", prefix, steps);
  }

  text_layer_set_text(layer, s_buffer);
}

/* Update screen with values in static variable. */
void update() {
  static char count_buffer[16];
  char * count_prefix = "";
  update_steps_text(count_buffer, sizeof(count_buffer),
    s_step_count, count_prefix, title_layer);

  static char goal_buffer[16];
  char * goal_prefix = "Goal: ";
  update_steps_text(goal_buffer, sizeof(goal_buffer),
    s_step_goal, goal_prefix, subtitle_layer);

  layer_mark_dirty(s_progress_layer);
}

/* Set static variables, and update screen. */
void main_window_update_steps(int step_count, int step_goal, int step_average) {
    s_step_count = step_count;
    s_step_goal = step_goal;
    s_step_average = step_average;
    update();
}

/* Create a window and push to the window stack. */
void main_window_push() {
  s_window = window_create();

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load  = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

/* Pop window from the window stack. */
void main_window_remove() {
  window_stack_remove(s_window, false);
}