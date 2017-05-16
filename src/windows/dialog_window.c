#include "dialog_window.h"

static Window *s_window;
static TextLayer *s_text_layer;
static ScrollLayer *s_scroll_layer;
static ContentIndicator *s_indicator;
static Layer *s_indicator_up_layer, *s_indicator_down_layer;


static int s_offset = 10;

/** 
 * Update the text displayed on this dialog window.
 */
void dialog_text_layer_update_proc(char *text) {
  text_layer_set_text(s_text_layer, text);

  // Set up ScrollLayer according to the text size (assuming top_text_layer_update_proc done).
  GSize text_size = text_layer_get_content_size(s_text_layer);

  GSize temp = scroll_layer_get_content_size(s_scroll_layer);
  
  GPoint gp = scroll_layer_get_content_offset(s_scroll_layer);
#if DEBUG
  APP_LOG(APP_LOG_LEVEL_ERROR, "scroll_size.w=%d, h=%d", temp.w, temp.h);
  APP_LOG(APP_LOG_LEVEL_ERROR, "main_text_size.w=%d, h=%d", text_size.w, text_size.h);
  APP_LOG(APP_LOG_LEVEL_ERROR, "absz.w=%d, h=%d", gp.x, gp.y);
#endif
  //text_size.h += 2 * s_offset;
  text_size.h += 100;
  scroll_layer_set_content_size(s_scroll_layer, text_size);

  #if defined(PBL_ROUND)
    text_layer_enable_screen_text_flow_and_paging(s_text_layer, 5);
  #endif
}

/* Back button click handler. Set the exit reason and then exit. */
static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop_all(false);
}

/**
 * Handler for the select button. It is same as back_click_handler() or used for debugging.
 */
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  back_click_handler(recognizer, context);
}

/* Set click event handlers. */
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/**
 * Create the window and push to the window stack. 
 */
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  GRect bounds = layer_get_bounds(window_layer);
#if DEBUG
  APP_LOG(APP_LOG_LEVEL_ERROR, "main_text_size.w=%d, h=%d", bounds.size.w, bounds.size.h);
#endif

  int text_layer_height = 800;



  // Create ScrollLayer.
  s_scroll_layer = scroll_layer_create(bounds);
  scroll_layer_set_shadow_hidden(s_scroll_layer, true);

  // Let the ScrollLayer receive click events, and set click handlers for windows.
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_set_callbacks(s_scroll_layer, (ScrollLayerCallbacks) {
    .click_config_provider =  click_config_provider,
  });

  // will change the scroll offset by the frame's height.
  scroll_layer_set_paging(s_scroll_layer, true);

  // Add the ScrollLayer to the main window layer.
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));



  // Get the ContentIndicator from the ScrollLayer
  s_indicator = scroll_layer_get_content_indicator(s_scroll_layer);

  // Create two Layers to draw the arrows
  s_indicator_up_layer = layer_create(GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  s_indicator_down_layer = layer_create(GRect(0, bounds.size.h - STATUS_BAR_LAYER_HEIGHT,
                                              bounds.size.w, STATUS_BAR_LAYER_HEIGHT));

  // Configure the properties of each indicator
  const ContentIndicatorConfig up_config = (ContentIndicatorConfig) {
    .layer = s_indicator_up_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = GColorBlack,
      .background = GColorWhite
    }
  };
  content_indicator_configure_direction(s_indicator, ContentIndicatorDirectionUp,
                                        &up_config);
  
  const ContentIndicatorConfig down_config = (ContentIndicatorConfig) {
    .layer = s_indicator_down_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = GColorBlack,
      .background = GColorWhite
    }
  };
  content_indicator_configure_direction(s_indicator, ContentIndicatorDirectionDown,
                                        &down_config);

  // Add indicator Layers as children of the main window layer.
  layer_add_child(window_layer, s_indicator_up_layer);
  layer_add_child(window_layer, s_indicator_down_layer);


  // Create TextLayer.
  GRect text_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, text_layer_height);
  #if defined(PBL_ROUND)
    GEdgeInsets text_insets = {.top = 0, .bottom = 0, .right = 0, .left = 0};
  #else
    GEdgeInsets text_insets = {.top = 0, .right = 2, .left = 2};
  #endif
  s_text_layer = text_layer_create(grect_inset(text_bounds, text_insets));
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_color(s_text_layer, GColorRed);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  // Add TextLayer as children of the ScrollLayer.
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  if (s_text_layer) {
    text_layer_destroy(s_text_layer);
    s_text_layer = NULL;
  }
  if (s_indicator_up_layer) {
    layer_destroy(s_indicator_up_layer);
    s_indicator_up_layer = NULL;
  }
  if (s_indicator_up_layer) {
    layer_destroy(s_indicator_down_layer);
    s_indicator_down_layer = NULL;
  }


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
