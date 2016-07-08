#include "main_window.h"

static Window *s_window;
static TextLayer *s_text_layer;

static TextLayer* make_text_layer(GRect bounds, GFont font) {
  TextLayer *this = text_layer_create(bounds);
  text_layer_set_background_color(this, GColorClear);
  text_layer_set_text_alignment(this, GTextAlignmentCenter);
  text_layer_set_font(this, font);
  return this;
}

static void window_load(Window * window) {
  // Get the bounds of the root layer
  Layer *root_layer  = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  // Create a text layer, and set the text
  s_text_layer = make_text_layer(bounds, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_text_layer, "hi!");

  // Add text layer to the window
  layer_add_child(root_layer, text_layer_get_layer(s_text_layer));

  // Enable text flow and paging on the text layer, with a slight inset of 10, for round screens
  text_layer_enable_screen_text_flow_and_paging(s_text_layer, 10);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  window_destroy(s_window);
}

void main_window_push() {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load  = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}