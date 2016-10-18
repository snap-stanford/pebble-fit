#include "display_func.h"
//  RAM SIZE : at least 800B
// HEAP MEMORY : 396B

//  start @ 11:11pm July 21, finished an hour later
static Window *s_reminder_window;

static TextLayer *s_detail_comment_text_layer;
static TextLayer *s_detail_text_layer;
static TextLayer *s_title_text_layer;
static TextLayer *s_reminder_text_layer;

// static BitmapLayer *s_BT_icon_layer;
// static GBitmap *s_BT_icon_bitmap;

static int16_t remind_code_lcl;


static void select_click_handler(ClickRecognizerRef recognizer, void *context){}

static void back_click_handler(ClickRecognizerRef recognizer, void *context){
  // DISMISS the reminder
  window_stack_remove(s_reminder_window, false);
  psleep(100); // wait a bit for the OS to reset the window pointers
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context){
  // nothing yet
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context){

}


static void click_config_provider(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void bluetooth_event_handler(bool connected){
  if(connected){
    // if CONNECTED BT, then get rid of the window.
    window_stack_remove(s_reminder_window, false);
    psleep(50); // wait a bit for the OS to reset the window pointers
  }
}

static void window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);




  s_title_text_layer = text_layer_create(
    GRect(11,0,window_bounds.size.w ,35));
  text_layer_set_text_alignment(s_title_text_layer, GTextAlignmentLeft);
  text_layer_set_font(s_title_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_title_text_layer," Memory Low");

  s_reminder_text_layer = text_layer_create(
    GRect(17,36,window_bounds.size.w ,90));
  text_layer_set_text_alignment(s_reminder_text_layer, GTextAlignmentLeft);
  text_layer_set_font(s_reminder_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_reminder_text_layer,"Turn On BT\nRestart Phone");


  s_detail_comment_text_layer = text_layer_create(
    GRect(0,100,window_bounds.size.w ,30));
  text_layer_set_text_alignment(s_detail_comment_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_detail_comment_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_detail_comment_text_layer, GColorBlack);
  text_layer_set_text_color(s_detail_comment_text_layer, GColorWhite);
  text_layer_set_text(s_detail_comment_text_layer,"Trouble? Help:\n");

  s_detail_text_layer = text_layer_create(
    GRect(0,100+30,window_bounds.size.w ,38));
  text_layer_set_text_alignment(s_detail_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_detail_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_background_color(s_detail_text_layer, GColorBlack);
  text_layer_set_text_color(s_detail_text_layer, GColorWhite);
  text_layer_set_text(s_detail_text_layer,"goo.gl/GNkZA6");
  // goo.gl/f3V8fy = http://projectkraepelin.org/pebble/help

  layer_add_child(window_layer,text_layer_get_layer(s_title_text_layer));
  layer_add_child(window_layer,text_layer_get_layer(s_reminder_text_layer));
  // add the text layers to the window
  layer_add_child(window_layer,text_layer_get_layer(s_detail_comment_text_layer));
  layer_add_child(window_layer,text_layer_get_layer(s_detail_text_layer));


  // // ADD BITMAPS
  // s_BT_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_ICON_48);
  // // Create the output Graphics layers
  // // s_BT_icon_layer = bitmap_layer_create(GRect(110,60,24,24));
  // s_BT_icon_layer = bitmap_layer_create(GRect(96,40,48,48));
  // // add the bitmaps to the layers
  // bitmap_layer_set_bitmap(s_BT_icon_layer,s_BT_icon_bitmap);
  // // add bitmap layers to the screen
  //
  // // PROLEM, KEEPS CRASHING HERE!!
  // layer_add_child(window_layer,bitmap_layer_get_layer(s_BT_icon_layer));
  APP_LOG(APP_LOG_LEVEL_ERROR, "reminder  heap size: used %d , free %d", heap_bytes_used(), heap_bytes_free());

}

static void window_unload(Window *window){
  text_layer_destroy(s_title_text_layer);
  text_layer_destroy(s_detail_comment_text_layer);
  text_layer_destroy(s_detail_text_layer);

  text_layer_destroy(s_reminder_text_layer);

  // gbitmap_destroy(s_BT_icon_bitmap);
  // bitmap_layer_destroy(s_BT_icon_layer);

  bluetooth_connection_service_unsubscribe();
}

void display_reminder(enum ReminderReason remind_code){
  remind_code_lcl = remind_code;
  // register the bluetooth connection service
  bluetooth_connection_service_subscribe(bluetooth_event_handler);

  // vibrate to remind the person to remind
  vibes_enqueue_custom_pattern(pinteract_vibe_pat);

  // ONLY if the reminder window does not yet exist do we add it
  if( !window_stack_contains_window(s_reminder_window)){
    // create window
    s_reminder_window = window_create();

    // use global variables to this file for sanity
    // set the window handlers
    window_set_window_handlers(s_reminder_window, (WindowHandlers){
      .load = window_load,
      .unload = window_unload,
    });

      // cause this function is not defined in basalt, cause it doesn't have status bar
    #ifdef PBL_SDK_2
      window_set_fullscreen(s_reminder_window,true);
    #endif

    window_set_click_config_provider(s_reminder_window,
        (ClickConfigProvider) click_config_provider);

    // push the window
    window_stack_push(s_reminder_window,false);
  }
}
