#include "main_window.h"

static Window *s_window;
static TextLayer *title_layer;
static TextLayer *subtitle_layer;

static TextLayer* make_text_layer(GRect bounds, GFont font) {
  TextLayer *this = text_layer_create(bounds);
  text_layer_set_background_color(this, GColorClear);
  text_layer_set_text_color(this, GColorBlack);
  text_layer_set_text_alignment(this, GTextAlignmentCenter);
  text_layer_set_font(this, font);
  return this;
}

static void window_load(Window * window) {
  // Get the bounds of the root layer
  Layer *root_layer  = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);
  float center = bounds.size.h /= 2;

  // Create a text layer, and set the text
  int height = 30;
  title_layer = make_text_layer(GRect(0, center - height, bounds.size.w, height), fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(title_layer, "Pebble Fit");

  subtitle_layer = make_text_layer(GRect(0, center, bounds.size.w, center + height), fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text(subtitle_layer, "A Better Today");


  // Add text layer to the window
  layer_add_child(root_layer, text_layer_get_layer(title_layer));
    layer_add_child(root_layer, text_layer_get_layer(subtitle_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(title_layer);
  window_destroy(s_window);
}

void main_window_update_steps(int steps) {
  if(steps > 0) {
    static char s_buffer[32];
    snprintf(s_buffer, sizeof(s_buffer), "%d steps today", steps);
    text_layer_set_text(subtitle_layer, s_buffer);
  } else {
    text_layer_set_text(subtitle_layer, "No steps today");
  }
}

void main_window_push() {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load  = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

void main_window_remove() {
  window_stack_remove(s_window, false);
}