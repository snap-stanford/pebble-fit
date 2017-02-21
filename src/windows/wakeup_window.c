#include "wakeup_window.h"

static Window *s_window;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_main_text_layer, *s_top_text_layer;
static GFont s_main_text_font, s_top_text_font;

static int s_steps;
static char s_start[12], s_end[12];
static char s_top_text_buf[40];
static char s_main_text_buf[512];
static int s_inactive_mins;

/* Set standard attributes on new text layer in this window. */
static TextLayer* make_text_layer(GRect bounds, GFont font, GTextAlignment align) {
  TextLayer *this = text_layer_create(bounds);
  text_layer_set_overflow_mode(this, GTextOverflowModeWordWrap);
  text_layer_set_background_color(this, GColorClear);
  text_layer_set_text_color(this, GColorBlack);
  text_layer_set_text_alignment(this, align);
  text_layer_set_font(this, font);
  return this;
}

/* Procedure for how to update s_top_text_layer. */
//static void top_text_layer_update_proc(Layer *layer, GContext *ctx) {
static void top_text_layer_update_proc() {
  char *text;
  if (e_launch_reason == LAUNCH_WAKEUP_PERIOD) {
    if (steps_get_pass()) {
      text = "Break accomplished. Nice work!";
    } else {
      text = "Opps! Break missed.";
    }
    snprintf(s_top_text_buf, sizeof(s_top_text_buf), "%s", text);
    text_layer_set_text(s_top_text_layer, s_top_text_buf);
  } else {
    //text = "";
    // DEBUG BEGIN
    if (!steps_get_pass()) {
      snprintf(s_top_text_buf, sizeof(s_top_text_buf), 
               "sedentary %d steps during \n%s-%s", s_steps, s_start, s_end);
      text_layer_set_text(s_top_text_layer, s_top_text_buf);
    } else {
      snprintf(s_top_text_buf, sizeof(s_top_text_buf), 
               "non-sedentary during \n%s-%s", s_start, s_end);
      text_layer_set_text(s_top_text_layer, s_top_text_buf);
    }
    // DEBUG END
  }

  //text_layer_set_text(s_top_text_layer, s_top_text_buf);
}

/* Procedure for how to update s_main_text_layer. */
//static void main_text_layer_update_proc(Layer *layer, GContext *ctx) {
static void main_text_layer_update_proc() {
  //if (e_launch_reason == LAUNCH_WAKEUP_NOTIFY) {
  if (e_launch_reason == LAUNCH_WAKEUP_NOTIFY || e_launch_reason == LAUNCH_PHONE) {
    // FIXME: For debugging purpose, display indicator for bluetooth connection
    const char *text;
    HealthActivityMask activity = health_service_peek_current_activities();
    switch(activity) {
      case HealthActivityNone:
        text = "None"; break;
      case HealthActivitySleep: 
      case HealthActivityRestfulSleep: 
        text = "Sleeping"; break;
      case HealthActivityWalk:
        text = "Walking"; break;
      case HealthActivityRun:
        text = "Running"; break;
      default:
        text = "Unknown";
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "launch_set_random_message,content=%s",launch_get_random_message());
    if (connection_service_peek_pebble_app_connection()) {
      snprintf(s_main_text_buf, sizeof(s_main_text_buf), "%s!%s!", launch_get_random_message(), text);
    } else {
      snprintf(s_main_text_buf, sizeof(s_main_text_buf), "%s.%s.", launch_get_random_message(), text);
    }
  } else {
    const char *daily_summary = enamel_get_message_daily_summary();
     
APP_LOG(APP_LOG_LEVEL_ERROR, enamel_get_total_hour());
    snprintf(s_main_text_buf, sizeof(s_main_text_buf), daily_summary, 
      store_read_break_count(), atoi(enamel_get_total_hour()));
  }
    
  text_layer_set_text(s_main_text_layer, s_main_text_buf);

  // Set up ScrollLayer according to the text size (assuming top_text_layer_update_proc done).
  strcat(s_main_text_buf, "\n\n\n");
  GSize text_size = text_layer_get_content_size(s_main_text_layer);
  GSize text_size2 = text_layer_get_content_size(s_top_text_layer);
  text_size.w += text_size2.w;
  text_size.h += text_size2.h;
  scroll_layer_set_content_size(s_scroll_layer, text_size);
}

/**
 * Make sure to update the top text TextLayer before the main TextLayer. 
 */
static void prv_update_text() {
  top_text_layer_update_proc();
  main_text_layer_update_proc();
}

/* Get the latest step count and update texts on the watch accordingly. */
void wakeup_window_breathe() {
  steps_update();
  steps_wakeup_window_update();
  prv_update_text();
}

/* Back button click handler. Set the exit reason and then exit. */
static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  e_exit_reason = EXIT_USER;
  window_stack_pop_all(false);
}

/**
 * Handler for the select button. It is same as back_click_handler() or used for debugging.
 */
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(s_main_text_layer, "reset timestamp");
  //store_write_upload_time(e_launch_time);

  back_click_handler(recognizer, context);

  //store_reset_break_count();

  // Reset last update timestamp to 2 hour ago
  //store_write_config_time(time(NULL) - 2 * SECONDS_PER_DAY);

  //steps_get_prior_week();
}

/* Deprecated. Set click event handlers. */
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  //int padding = PBL_IF_ROUND_ELSE(10, 0);
  int padding = 10;
  float center = bounds.size.h / 2;
  int top_text_height = 40;
  //int main_text_height = PBL_IF_ROUND_ELSE(60, 70);
  int main_text_height = 2000;

  // The text layer that is above the main text layer
  GEdgeInsets top_text_insets = {.top = 10, .right = 10, .left = 10};
  s_top_text_font = fonts_get_system_font(FONT_KEY_GOTHIC_24); 
  s_top_text_layer = make_text_layer(
    grect_inset(bounds, top_text_insets),s_top_text_font, GTextAlignmentCenter);

  // The main text layer that resides in the center of the screen
  GEdgeInsets main_text_insets = {.top = 10, .right = 0, .left = 0};
  GRect main_text_bounds = grect_inset(
    GRect(0, center - padding, bounds.size.w, main_text_height), main_text_insets);
  s_main_text_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_main_text_layer = make_text_layer(main_text_bounds, s_main_text_font, GTextAlignmentCenter);

  // Create the ScrollLayer
  s_scroll_layer = scroll_layer_create(bounds);
  //scroll_layer_set_shadow_hidden(s_scroll_layer, true);

  // Let the ScrollLayer receive click events, and set click handlers for windows.
  scroll_layer_set_click_config_onto_window(s_scroll_layer, s_window);
  scroll_layer_set_callbacks(s_scroll_layer, (ScrollLayerCallbacks) {
    .click_config_provider =  click_config_provider,
  });

  // Set the update procedure
  prv_update_text();

  // Add TextLayer as children of the ScrollLayer.
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_top_text_layer));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_main_text_layer));

  // Add the ScrollLayer to the window.
  layer_add_child(root_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void window_unload(Window *window) {
  if (s_scroll_layer) {
    scroll_layer_destroy(s_scroll_layer);
    s_scroll_layer = NULL;
  }
  if (s_main_text_layer) {
    text_layer_destroy(s_main_text_layer);
    s_main_text_layer = NULL;
  }
  if (s_top_text_layer) {
    text_layer_destroy(s_top_text_layer);
    s_top_text_layer = NULL;
  }

  window_destroy(s_window);
  s_window = NULL;
}

/**
 *  Create a wakeup window and push to the window stack. 
 *  wakeup_window_type specifies what kind of messages to be displayed.
 *    WAKEUP_WINDOW_MANUAL: manually launch message.
 *    WAKEUP_WINDOW_DAILY: daily message.
 *    WAKEUP_WINDOW_NOTIFY: notification message.
 *    WAKEUP_WINDOW_PERIOD: normal periodic wakeup message.
 */
Window * wakeup_window_push() {
  steps_wakeup_window_update();
  if (!s_window) {
    s_window = window_create();

    window_set_window_handlers(s_window, (WindowHandlers) {
      .load  = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
  return s_window;
}

/* Pop window from the window stack. */
void wakeup_window_remove() {
  window_stack_remove(s_window, false);
}

/* Called by the steps module to update the information related to step count. */
void wakeup_window_update(int steps, char *start, char *end, int inactive_mins) {
  s_steps = steps;
  strncpy(s_start, start, sizeof(s_start));
  strncpy(s_end, end, sizeof(s_end));
  s_inactive_mins = inactive_mins;
}


