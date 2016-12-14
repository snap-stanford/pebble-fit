#include "wakeup_window.h"

static time_t s_time;

static Window *s_window;
static TextLayer *s_main_text_layer, *s_top_text_layer, *s_bot_text_layer;

static int s_step;
static char s_start[12], s_end[12];
static char s_top_text_buf[40];
static char s_bot_text_buf[40];
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
  APP_LOG(APP_LOG_LEVEL_INFO, "Steps: %d", s_step);
  snprintf(s_top_text_buf, sizeof(s_top_text_buf), 
           "%d steps during \n%s-%s", s_step, s_start, s_end);
  text_layer_set_text(s_top_text_layer, s_top_text_buf);
}

/* Procedure for how to update s_main_text_layer. */
//static void main_text_layer_update_proc(Layer *layer, GContext *ctx) {
static void main_text_layer_update_proc() {
  if (s_step > enamel_get_step_threshold()) {
    if (connection_service_peek_pebble_app_connection()) 
        text_layer_set_text(s_main_text_layer, "Keep up!");
    else 
        text_layer_set_text(s_main_text_layer, "Keep up.");
  } else {
    if (connection_service_peek_pebble_app_connection()) 
        text_layer_set_text(s_main_text_layer, "Let's Move!");
    else
        text_layer_set_text(s_main_text_layer, "Let's Move.");
  }
}

/* Procedure for how to update s_bot_text_layer. */
//static void bot_text_layer_update_proc(Layer *layer, GContext *ctx) {
static void bot_text_layer_update_proc() {
  snprintf(s_bot_text_buf, sizeof(s_bot_text_buf), 
           "been inactive for the last %dmins", s_inactive_mins);
  text_layer_set_text(s_bot_text_layer, s_bot_text_buf);
}

/* Mark all text layers to be dirty so that they can be re-rendered. */
static void layer_mark_dirty_all() {
  layer_mark_dirty((Layer *)s_top_text_layer);
  layer_mark_dirty((Layer *)s_main_text_layer);
  layer_mark_dirty((Layer *)s_bot_text_layer);
}

static void prv_update_text() {
	top_text_layer_update_proc();
	main_text_layer_update_proc();
	bot_text_layer_update_proc();
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
  int main_text_height = 40;
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
  GEdgeInsets bot_text_insets = {.top = center+main_text_height/2, .right = 10, .left = 10};
  s_bot_text_layer = make_text_layer(
    grect_inset(bounds, bot_text_insets),
    fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentCenter);

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
  layer_add_child(root_layer, text_layer_get_layer(s_bot_text_layer));

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

/* Set click event handlers. */
static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

/* Create a window and push to the window stack. */
Window * wakeup_window_push() {
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
  s_step = steps;
  strncpy(s_start, start, sizeof(s_start));
  strncpy(s_end, end, sizeof(s_end));
  s_inactive_mins = inactive_mins;
}


