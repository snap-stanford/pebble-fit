#include "wakeup_window.h"

static Window *s_window;
static TextLayer *s_main_text_layer, *s_top_text_layer;
static ScrollLayer *s_scroll_layer;
static ContentIndicator *s_indicator;
static Layer *s_indicator_up_layer, *s_indicator_down_layer;
static GFont s_main_text_font, s_top_text_font;

static int s_steps;
static char s_start[12], s_end[12];
static char s_top_text_buf[64];
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
static void top_text_layer_update_proc() {
#if DEBUG
  APP_LOG(APP_LOG_LEVEL_ERROR, "enter top_text_layer_update_proc()");
#endif
  const char *text;

  // Display a warning wakeup to notify the user of losing connection for 36 hours.
  if (!connection_service_peek_pebble_app_connection() && 
      store_read_upload_time() < e_launch_time - 36 * SECONDS_PER_HOUR) {
    text = "Launch Pebble app to synch!";
    snprintf(s_top_text_buf, sizeof(s_top_text_buf), "%s", text);
  } else if (e_launch_reason == LAUNCH_WAKEUP_PERIOD) {
    if (steps_get_pass()) {
      text = enamel_get_message_pass();
    } else {
      text = enamel_get_message_fail();
    }
    snprintf(s_top_text_buf, sizeof(s_top_text_buf), "%s", text);
  } else { 
    // Other launch event, supposed to display nothing in this top TextLayer.
    text = "";

    // Testing 
      //text = "Launch Pebble app to synch!";
      //text = "Break accomplished. Nice work!";
      text = "Opps! Break missed.";

    snprintf(s_top_text_buf, sizeof(s_top_text_buf), "%s", text);

    // DEBUG BEGIN
/*
    if (!steps_get_pass()) {
      snprintf(s_top_text_buf, sizeof(s_top_text_buf), 
               "sedentary %d steps during \n%s-%s", s_steps, s_start, s_end);
    } else {
      snprintf(s_top_text_buf, sizeof(s_top_text_buf), 
               "non-sedentary during \n%s-%s", s_start, s_end);
    }
*/

    // DEBUG END
  }

  text_layer_set_text(s_top_text_layer, s_top_text_buf);
}

/* Procedure for how to update s_main_text_layer. */
static void main_text_layer_update_proc() {
#if DEBUG
  APP_LOG(APP_LOG_LEVEL_ERROR, "enter main_text_layer_update_proc()");
#endif

  if (e_launch_reason == LAUNCH_WAKEUP_ALERT) {
  //if (e_launch_reason == LAUNCH_WAKEUP_ALERT || e_launch_reason == LAUNCH_PHONE) { // DEBUG
  //  launch_set_random_message(); // DEBUG
    snprintf(s_main_text_buf, sizeof(s_main_text_buf), "%s", launch_get_random_message());
  } else {
    const char *message_summary = enamel_get_message_summary();

    snprintf(s_main_text_buf, sizeof(s_main_text_buf), message_summary, 
      store_read_curr_score(), store_read_possible_score());
      //store_read_curr_score(), enamel_get_total_break());
      
    //TODO: workaround for scrolling issue for some long text.
    #if defined(PBL_ROUND)
      strcat(s_main_text_buf, "\n\n");
    #else
      strcat(s_main_text_buf, "\n");
    #endif
  }  

  text_layer_set_text(s_main_text_layer, s_main_text_buf);

  // Set up ScrollLayer according to the text size (assuming top_text_layer_update_proc done).
  GSize top_text_size = text_layer_get_content_size(s_top_text_layer);
  GSize main_text_size = text_layer_get_content_size(s_main_text_layer);
  #if DEBUG
  APP_LOG(APP_LOG_LEVEL_ERROR, "main_text_size.w=%d, h=%d", main_text_size.w, main_text_size.h);
  #endif
  main_text_size.w += top_text_size.w;
  main_text_size.h += top_text_size.h+5;
  #if DEBUG
  APP_LOG(APP_LOG_LEVEL_ERROR, "Gsize height = %d", main_text_size.h);
  #endif

  scroll_layer_set_content_size(s_scroll_layer, main_text_size);
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
  APP_LOG(APP_LOG_LEVEL_INFO, "DEBUG: wakeup_window_breathe.");
  //steps_update();
  //steps_wakeup_window_update();
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
  #if DEBUG
  // DEBUG: step data re-send.
  //text_layer_set_text(s_main_text_layer, "reset timestamp");
  //store_write_upload_time(e_launch_time - 2 * SECONDS_PER_DAY);

  // DEBUG: reset launchexit count.
  int temp = 0;
  persist_write_data(PERSIST_KEY_LAUNCHEXIT_COUNT, &temp, 1);
  store_write_upload_time(e_launch_time - SECONDS_PER_HOUR);
  
  // DEBUG: random messages.
  launch_set_random_message();
  snprintf(s_main_text_buf, sizeof(s_main_text_buf), "%s", launch_get_random_message());
  text_layer_set_text(s_main_text_layer, s_main_text_buf);

  // DEBUG: current score.
  //store_reset_curr_score();

  // DEBUG: reset last update timestamp to 2 hour ago
  //store_write_config_time(time(NULL) - 2 * SECONDS_PER_DAY);

  // DEBUG: send prior week's data
  //steps_upload_prior_week();
  #else
    back_click_handler(recognizer, context);
  #endif
}

/* Set click event handlers. */
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  //APP_LOG(APP_LOG_LEVEL_ERROR, "bound height = %d", bounds.size.h);

  //int padding = PBL_IF_ROUND_ELSE(10, 0);
  int padding = 10;
  float center = bounds.size.h / 2;
  int top_text_height = 40;
  //int main_text_height = PBL_IF_ROUND_ELSE(60, 70);
  int main_text_height = 2000;


  // Create the ScrollLayer.
  s_scroll_layer = scroll_layer_create(bounds);
  scroll_layer_set_shadow_hidden(s_scroll_layer, true);

  // Let the ScrollLayer receive click events, and set click handlers for windows.
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_set_callbacks(s_scroll_layer, (ScrollLayerCallbacks) {
    .click_config_provider =  click_config_provider,
  });

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



  // Create the top TextLayer that is above the main TextLayer.
  s_top_text_font = fonts_get_system_font(FONT_KEY_GOTHIC_24); 
  s_top_text_layer = make_text_layer(bounds, s_top_text_font, GTextAlignmentCenter);

  // Add TextLayer as children of the ScrollLayer.
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_top_text_layer));

  // Enable paging and text flow with an inset of 5 pixels
  text_layer_enable_screen_text_flow_and_paging(s_top_text_layer, 5);

  top_text_layer_update_proc();



  // Create the main TextLayer that display after the top TextLayer.
  // Note: due to the fact that text_layer_get_content_size() does no give accurate value when
  // used with text_layer_enable_screen_text_flow_and_paging(), we add an arbitrary margin
  // when creating the main TextLayer.
  
  // TODO: We need to get a sense of the size of message to place the text layer properly 
  // approximately at the center of the screen.
  #if DEBUG
  int temp_length;
  if (e_launch_reason == LAUNCH_WAKEUP_ALERT) {
    temp_length = strlen(launch_get_random_message());
  } else {
    temp_length = strlen(launch_get_random_message());
  }
  #endif
/*
  if (e_launch_reason == LAUNCH_WAKEUP_ALERT) {
  //if (e_launch_reason == LAUNCH_WAKEUP_ALERT || e_launch_reason == LAUNCH_PHONE) { // DEBUG
    launch_set_random_message();
    APP_LOG(APP_LOG_LEVEL_ERROR, "random_message,content=%s", launch_get_random_message());
    snprintf(s_main_text_buf, sizeof(s_main_text_buf), "%s", launch_get_random_message());
    //snprintf(s_main_text_buf, sizeof(s_main_text_buf), "Nice outside? Talk a stroll.");
    //snprintf(s_main_text_buf, sizeof(s_main_text_buf), "regular walking breaks can reduce risk of cardiovascular disease");
  } else {
    const char *daily_summary = enamel_get_message_summary();
     
    snprintf(s_main_text_buf, sizeof(s_main_text_buf), daily_summary, 
      store_read_curr_score(), enamel_get_total_break());

    strcat(s_main_text_buf, "\n\n\n"); //TODO: workaround for scrolling issue for some long text.
  }
*/
  #if DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "string length=%d", temp_length);
  #endif
 
  GSize top_text_size = text_layer_get_content_size(s_top_text_layer);
  GRect main_bounds = GRect(bounds.origin.x, bounds.origin.y + top_text_size.h, 
                            bounds.size.w, bounds.size.h); // TODO: different height value?

  #if defined(PBL_ROUND)
    GEdgeInsets main_text_insets = {.top = (top_text_size.h > 0)? 5 : 30, 
                                    .right = 20, .left = 20};
  #else
    GEdgeInsets main_text_insets = {.top = (top_text_size.h > 0)? 5 : 25, .right = 0, .left = 0};
  #endif
  s_main_text_font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  //s_main_text_layer = make_text_layer(main_bounds, s_main_text_font, GTextAlignmentCenter);
  //s_main_text_layer = make_text_layer(grect_inset(main_bounds, GEdgeInsets(15)), 
  s_main_text_layer = make_text_layer(grect_inset(main_bounds, main_text_insets), 
                                      s_main_text_font, GTextAlignmentCenter);

  // Add TextLayer as children of the ScrollLayer.
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_main_text_layer));

  // Enable paging and text flow with an inset of 5 pixels
  // Note that if there is too much text in the top TextLayer, this pagination might cause 
  // unneccessary line spaces and clipped texts.
  if (top_text_size.h == 0) {
    text_layer_enable_screen_text_flow_and_paging(s_main_text_layer, 5);
  }

  main_text_layer_update_proc();
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
  if (s_indicator_up_layer) {
    layer_destroy(s_indicator_up_layer);
    s_indicator_up_layer = NULL;
  }
  if (s_indicator_up_layer) {
    layer_destroy(s_indicator_down_layer);
    s_indicator_down_layer = NULL;
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
  //steps_wakeup_window_update();
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


