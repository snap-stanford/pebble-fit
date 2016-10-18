
//  RAM SIZE : nearly 1 KB. Is this really worth it?
#include "display_func.h"


// BT handler, to update when it works
// click handlers. Don't really need to go back to the reminder page
// scroll layer
// title layer
// BT icon layer
//

static Window *s_help_window;

static TextLayer *s_BT_status_text_layer;
static TextLayer *s_help_text_layer;

static ScrollLayer *s_help_scroll_layer;

static BitmapLayer *s_BT_icon_bitmap_layer;
static BitmapLayer *s_BT_status_bitmap_layer;
static BitmapLayer *s_scroll_text_bitmap_layer;


static GBitmap *s_BT_icon_bitmap;
static GBitmap *s_BT_status_Xmark_bitmap;
static GBitmap *s_BT_status_Checkmark_bitmap;
static GBitmap *s_scroll_text_bitmap;

/*

// static char s_help_scroll_text[]= " Please attempt procedures in order. If\
//   Bluetooth (BT) Status icon is a checkmark, you are done. \n\n\
//   > Procedure 1 : \n\
//   1. Disable Airplane Mode\n\n\
//   > Procedure 2 : \n\
//   1. phone Settings\n\
//   2. Turn BT ON\n\n\
//   > Procedure 3 : \n\
//   1. phone Settings\n\
//   2. Turn BT OFF\n\
//   3. Wait 5 seconds\n\
//   4. Turn BT ON\n\
//   5. Wait 10 seconds\n\n\
//   > Procedure 4\n\
//   1. Reset phone\n\
//   2. phone Settings\n\
//   3. Turn BT ON\n\
//   4. Wait 10 seconds \n\n\
//   If after above procedures you still see an X-mark next to BT Status, then\
//   please email \n\nproject.kraepelin.contact@gmail.com\n\n\
//   Thank you.";
*/

static void bluetooth_event_handler(bool connected){
  if(connected){
    // if CONNECTED BT
    bitmap_layer_set_bitmap(s_BT_status_bitmap_layer, s_BT_status_Checkmark_bitmap);

  }else{
    // if NOT connected BT
    bitmap_layer_set_bitmap(s_BT_status_bitmap_layer, s_BT_status_Xmark_bitmap);
  }
  layer_mark_dirty((Layer*) s_BT_status_bitmap_layer);
}


static void window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // static char s_BT_status_text_buf[12];
  int8_t icon_size = 24;
  int8_t ofs = 3;
  // create the relevant bitmaps
  s_BT_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_ICON_24);
  s_BT_status_Xmark_bitmap = gbitmap_create_with_resource(RESOURCE_ID_X_MARK_24);
  s_BT_status_Checkmark_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CHECK_MARK_24);
  s_scroll_text_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_CONNECT_HELP_TEXT_144_750);
  // create the bitmap layers
  s_BT_icon_bitmap_layer = bitmap_layer_create(GRect(ofs,ofs,icon_size,icon_size));
  s_BT_status_bitmap_layer = bitmap_layer_create(
    GRect(window_bounds.size.w - icon_size - ofs,ofs,icon_size,icon_size));
  GSize scroll_text_bitmap_layer_size = GSize(144,750);
  s_scroll_text_bitmap_layer = bitmap_layer_create(GRect(0,0,
    scroll_text_bitmap_layer_size.w, scroll_text_bitmap_layer_size.h));

  // add the bitmaps to the layers
  bitmap_layer_set_bitmap(s_BT_icon_bitmap_layer,s_BT_icon_bitmap);
  bitmap_layer_set_bitmap(s_scroll_text_bitmap_layer,s_scroll_text_bitmap);
  // check current BT state to set the status icon
  bluetooth_event_handler(bluetooth_connection_service_peek());
  // add bitmap layers to the screen
  layer_add_child(window_layer,bitmap_layer_get_layer(s_BT_icon_bitmap_layer));
  layer_add_child(window_layer,bitmap_layer_get_layer(s_BT_status_bitmap_layer));



  // BT STATUS text layer
  s_BT_status_text_layer = text_layer_create(
    GRect(icon_size + ofs,-3,window_bounds.size.w - 2*(icon_size+ofs),icon_size));
  text_layer_set_text_alignment(s_BT_status_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_BT_status_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_BT_status_text_layer, "BT Status:");
  layer_add_child(window_layer, text_layer_get_layer(s_BT_status_text_layer));


  // Help Scroll Layer
  s_help_scroll_layer = scroll_layer_create(
    GRect(0,icon_size + ofs,window_bounds.size.w,
      window_bounds.size.h-(icon_size + ofs)));
  scroll_layer_set_click_config_onto_window(s_help_scroll_layer, window);
  // Trim text layer and scroll content to fit text box
  scroll_layer_set_content_size(s_help_scroll_layer,
    GSize(scroll_text_bitmap_layer_size.w, scroll_text_bitmap_layer_size.h + 10));
  // add the text layer to the scroll layer
  scroll_layer_add_child(s_help_scroll_layer, bitmap_layer_get_layer(s_scroll_text_bitmap_layer));
  // add the scroll layer to the main window
  layer_add_child(window_layer, scroll_layer_get_layer(s_help_scroll_layer));


  // // Help text layer
  // GRect max_help_text_bounds = GRect(2,0,window_bounds.size.w-4,2020);
  // s_help_text_layer = text_layer_create(max_help_text_bounds);
  // text_layer_set_text(s_help_text_layer, s_help_scroll_text);
  // text_layer_set_font(s_help_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  // GSize max_help_text_size = text_layer_get_content_size(s_help_text_layer);
  // text_layer_set_size(s_help_text_layer, max_help_text_size);
  //
  // // Help Scroll Layer
  // s_help_scroll_layer = scroll_layer_create(
  //   GRect(0,icon_size + ofs,window_bounds.size.w,
  //     window_bounds.size.h-(icon_size + ofs)));
  // scroll_layer_set_click_config_onto_window(s_help_scroll_layer, window);
  // // Trim text layer and scroll content to fit text box
  // scroll_layer_set_content_size(s_help_scroll_layer,
  //   GSize(window_bounds.size.w, max_help_text_size.h + 10));
  // // add the text layer to the scroll layer
  // scroll_layer_add_child(s_help_scroll_layer, text_layer_get_layer(s_help_text_layer));
  // // add the scroll layer to the main window
  // layer_add_child(window_layer, scroll_layer_get_layer(s_help_scroll_layer));

}

static void window_unload(Window *window){

  text_layer_destroy(s_BT_status_text_layer);
  text_layer_destroy(s_help_text_layer);

  scroll_layer_destroy(s_help_scroll_layer);

  bitmap_layer_destroy(s_BT_icon_bitmap_layer);
  bitmap_layer_destroy(s_BT_status_bitmap_layer);

  gbitmap_destroy(s_BT_icon_bitmap);
  gbitmap_destroy(s_BT_status_Xmark_bitmap);
  gbitmap_destroy(s_BT_status_Checkmark_bitmap);
  gbitmap_destroy(s_scroll_text_bitmap);

  bluetooth_connection_service_unsubscribe();
}


void display_BT_connect_help(){
  // register the bluetooth connection service
  bluetooth_connection_service_subscribe(bluetooth_event_handler);

  // set up the main window

  // create window
  s_help_window = window_create();

  // use global variables to this file for sanity
  // set the window handlers
  window_set_window_handlers(s_help_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });

    // cause this function is not defined in basalt, cause it doesn't have status bar
  #ifdef PBL_SDK_2
    window_set_fullscreen(s_help_window,true);
  #endif


  // push the window
  window_stack_push(s_help_window,false);
}
