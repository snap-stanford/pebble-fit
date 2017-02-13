#include "wakeup_window.h"

static bool s_wakeup_launch;
static time_t s_time;

static Window *s_window;
static TextLayer *s_main_text_layer, *s_top_text_layer, *s_bot_text_layer;

static int s_steps;
static char s_start[12], s_end[12];
static char s_top_text_buf[40];
static char s_bot_text_buf[40];
static char s_main_text_buf[128];
static int s_inactive_mins;
  

/* Set standard attributes on new text layer in this window. */
static TextLayer* make_text_layer(GRect bounds, GFont font, GTextAlignment align) {
  TextLayer *this = text_layer_create(bounds);
  text_layer_set_background_color(this, GColorClear);
  text_layer_set_text_color(this, GColorBlack);
  text_layer_set_text_alignment(this, align);
  text_layer_set_font(this, font);
  return this;
}

/* Procedure for how to update s_top_text_layer. */
//static void top_text_layer_update_proc(Layer *layer, GContext *ctx) {
static void top_text_layer_update_proc() {
// Testing-begin
  const char *text;
  HealthActivityMask activity = health_service_peek_current_activities();
  switch(activity) {
    case HealthActivityNone:
      text = "No activity."; break;
    case HealthActivitySleep: 
    case HealthActivityRestfulSleep: 
      text = "Sleeping activity."; break;
    case HealthActivityWalk:
      text =  "Walking activity."; break;
    case HealthActivityRun:
      text = "Running activity."; break;
    default:
      text = "Unknown activity."; break;
  }

  // FIXME: For debugging purpose, display indicator for bluetooth connection
  if (connection_service_peek_pebble_app_connection()) {
    snprintf(s_top_text_buf, sizeof(s_top_text_buf), "%s!", text);
  } else {
    snprintf(s_top_text_buf, sizeof(s_top_text_buf), "%s.", text);
  }

  snprintf(s_top_text_buf, sizeof(s_top_text_buf), text);
  text_layer_set_text(s_top_text_layer, s_top_text_buf);
// Testing-end
  //if (!steps_get_pass()) {
  //  snprintf(s_top_text_buf, sizeof(s_top_text_buf), 
  //           "%d steps during \n%s-%s", s_steps, s_start, s_end);
  //  text_layer_set_text(s_top_text_layer, s_top_text_buf);
  //} else {
  //  snprintf(s_top_text_buf, sizeof(s_top_text_buf), 
  //           "non-sedentary during \n%s-%s", s_start, s_end);
  //  text_layer_set_text(s_top_text_layer, s_top_text_buf);
  //}
}

/* Procedure for how to update s_main_text_layer. */
//static void main_text_layer_update_proc(Layer *layer, GContext *ctx) {
static void main_text_layer_update_proc() {
  const char *text;

	if (s_wakeup_launch) {
    text = "Insert the random message here.";
    snprintf(s_main_text_buf, sizeof(s_main_text_buf), text);
	} else {
		text = enamel_get_daily_summary_message();
    snprintf(s_main_text_buf, sizeof(s_main_text_buf), text, 
        store_get_break_count(), atoi(enamel_get_total_hour()));
	}
  	
  text_layer_set_text(s_main_text_layer, s_main_text_buf);
}

/* Procedure for how to update s_bot_text_layer. */
//static void bot_text_layer_update_proc(Layer *layer, GContext *ctx) {
//static void bot_text_layer_update_proc() {
//  snprintf(s_bot_text_buf, sizeof(s_bot_text_buf), 
//           "been inactive for the last %dmins", s_inactive_mins);
//  text_layer_set_text(s_bot_text_layer, s_bot_text_buf);
//}

/* Mark all text layers to be dirty so that they can be re-rendered. */
static void layer_mark_dirty_all() {
  layer_mark_dirty((Layer *)s_top_text_layer);
  layer_mark_dirty((Layer *)s_main_text_layer);
  //layer_mark_dirty((Layer *)s_bot_text_layer);
}

static void prv_update_text() {
	top_text_layer_update_proc();
	main_text_layer_update_proc();
	//bot_text_layer_update_proc();
}

/* Get the latest step count and update texts on the watch accordingly. */
void wakeup_window_breathe() {
  steps_update();
  steps_wakeup_window_update();
	prv_update_text();
  //layer_mark_dirty_all();
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  //int padding = PBL_IF_ROUND_ELSE(10, 0);
  int padding = 10;
  float center = bounds.size.h / 2;
  int top_text_height = 40;
  //int main_text_height = PBL_IF_ROUND_ELSE(60, 70);
  int main_text_height = 80;
  int bot_text_height = 40;

  // The text layer that is above the main text layer
  GEdgeInsets top_text_insets = {.top = 20, .right = 10, .left = 10};
  s_top_text_layer = make_text_layer(
    grect_inset(bounds, top_text_insets),
    PBL_IF_ROUND_ELSE(fonts_get_system_font(FONT_KEY_GOTHIC_24), 
      fonts_get_system_font(FONT_KEY_GOTHIC_24)), 
    GTextAlignmentCenter);

  // The main text layer that resides in the center of the screen
  s_main_text_layer = make_text_layer(
    GRect(0, center - padding, bounds.size.w, main_text_height), 
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter);

  // The text layer that is below the main text layer
  //GEdgeInsets bot_text_insets = {.top = center+main_text_height/2, .right = 10, .left = 10};
  //s_bot_text_layer = make_text_layer(
  //  grect_inset(bounds, bot_text_insets),
  //  fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentCenter);

  // Set the update procedure
  //layer_set_update_proc((Layer *)s_top_text_layer, top_text_layer_update_proc);
  //layer_set_update_proc((Layer *)s_main_text_layer, main_text_layer_update_proc);
  //layer_set_update_proc((Layer *)s_bot_text_layer, bot_text_layer_update_proc);
	//top_text_layer_update_proc();
	//main_text_layer_update_proc();
	//bot_text_layer_update_proc();
  prv_update_text();

  // Add text layers to the window
  layer_add_child(root_layer, text_layer_get_layer(s_top_text_layer));
  layer_add_child(root_layer, text_layer_get_layer(s_main_text_layer));
  //layer_add_child(root_layer, text_layer_get_layer(s_bot_text_layer));

  // Mark them as dirty so that they can be rendered by update procedure immediately
  //layer_mark_dirty_all();
}

static void window_unload(Window *window) {
  if (s_main_text_layer) {
    text_layer_destroy(s_main_text_layer);
    s_main_text_layer = NULL;
  }
  if (s_top_text_layer) {
    text_layer_destroy(s_top_text_layer);
    s_top_text_layer = NULL;
  }
  if (s_bot_text_layer) {
    text_layer_destroy(s_bot_text_layer);
    s_bot_text_layer = NULL;
  }
  //if (s_debug1_layer) {
  //  text_layer_destroy(s_debug1_layer);
  //  s_debug1_layer = NULL;
  //}

  window_destroy(s_window);
  s_window = NULL;
}

/* Back button click handler. */
static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
	e_exit_reason = USER_EXIT;
	window_stack_pop_all(false);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_main_text_layer, "test mode 0");
  launch_send_test(0);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_main_text_layer, "reset timestamp");
  // Reset last update timestamp to 2 hour ago
  store_write_config_time(time(NULL) - 2 * SECONDS_PER_DAY);
  //steps_get_prior_week();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_main_text_layer, "test mode 1");
  launch_send_test(1);
}


/* Set click event handlers. */
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

/* Create a window and push to the window stack. */
Window * wakeup_window_push(bool is_wakeup_launch) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "is_wakeup_launch = %d", (int)is_wakeup_launch);
	s_wakeup_launch = is_wakeup_launch;
  steps_wakeup_window_update();
  if (!s_window) {
    s_window = window_create();

		// Set click handlers for windows.
  	window_set_click_config_provider(s_window, click_config_provider);


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


