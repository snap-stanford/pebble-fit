#include "dialog_window.h"

static Window *s_window;
static TextLayer *s_text_layer;

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);

  GRect bounds = layer_get_bounds(root_layer);

  float center = bounds.size.h / 2;
  int padding = 10;
  
  int title_height = 100;

  GEdgeInsets text_insets = {.top = center - title_height/2 - padding, .right = 10, .left = 10};
  s_text_layer = text_layer_create(grect_inset(bounds, text_insets));
  text_layer_set_background_color(s_text_layer, GColorClear);
	text_layer_set_text_color(s_text_layer, GColorRed);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_text_layer, "You must activate this app from the 'Settings' page on your phone.");


  // Add text layer to the window
  layer_add_child(root_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  window_destroy(s_window);
}

/* Create a window and push to the window stack. */
Window * dialog_window_push() {
  s_window = window_create();

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load  = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  return s_window;
}
