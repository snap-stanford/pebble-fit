#include "main_window.h"

static Window *s_window;
static TextLayer *title_layer;
static TextLayer *subtitle_layer;
static TextLayer *steps_layer;
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

/* Repaint the context with new (static var) values. */
static void progress_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));

  graphics_context_set_fill_color(ctx, 
    s_step_count >= s_step_average ? GColorJaegerGreen : GColorPictonBlue);

  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 16,
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
  text_layer_set_text(subtitle_layer, "Keep going!");

  steps_layer = make_text_layer(GRect(0, bounds.size.h - height, bounds.size.w, height), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GTextAlignmentRight);
  text_layer_set_text(steps_layer, "");

  // Add text layer to the window
  layer_add_child(root_layer, s_progress_layer);
  layer_add_child(root_layer, text_layer_get_layer(title_layer));
  layer_add_child(root_layer, text_layer_get_layer(subtitle_layer));
  layer_add_child(root_layer, text_layer_get_layer(steps_layer));
}

/* Destroy elements before destroying window. */
static void window_unload(Window *window) {
  text_layer_destroy(title_layer);
  text_layer_destroy(subtitle_layer);
  layer_destroy(s_progress_layer);
  window_destroy(s_window);
}

/* Update screen with values in static variable. */
void update() {
  static char s_current_steps_buffer[16];
  int thousands = s_step_count / 1000;
  int hundreds = s_step_count / 100;

  // Color based on progress
  bool isOnTrackForGoal = (s_step_count >= s_step_average);

  if(isOnTrackForGoal) {
    text_layer_set_text_color(steps_layer, GColorJaegerGreen);
  } else {
    text_layer_set_text_color(steps_layer, GColorPictonBlue);
  }

  if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d.%dk", thousands, hundreds);
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d", hundreds);
  }

  text_layer_set_text(steps_layer, s_current_steps_buffer);
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