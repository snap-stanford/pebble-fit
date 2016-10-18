#include "pinteract.h"
#include "pinteract_structs.h"


static Window *s_survey_window;

static MenuLayer *s_menu_layer;
static TextLayer *s_title_text_layer;
static int16_t init_selection;

Pinteract11Data pi11_res;


static uint16_t get_num_rows_cb(MenuLayer *menu_layer, uint16_t section_index, void *callback_context){
  return 5;
}

static uint16_t get_cell_height_cb(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
  return 26;
}

// we do not use the section header because the font size is only 14

static void select_click_cb(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context){
  // write the data to the struct
  pi11_res.time_end_pi = time(NULL);
  pi11_res.mood = 5 - (cell_index->row);

  // SET THE NEW PINTERACT STATE
  Pinteract11State pi11_state = {
    .mood_index = cell_index->row
  };
  set_pinteract_state(11, (void*) &pi11_state, 0);

  // write the data to persistant storage
  // get a new persistent storage key based on the current count of pinteracts stored
  int16_t new_pinteract_count = persist_read_int(PINTERACT_KEY_COUNT_PERSIST_KEY) + 1;
  // write the new data to persistant storage
  persist_write_data(new_pinteract_count, &pi11_res, sizeof(Pinteract11Data) );
  // update the pinteract storage count
  persist_write_int(PINTERACT_KEY_COUNT_PERSIST_KEY, new_pinteract_count);
  // close out the mood record menu entirely
  window_stack_remove(s_survey_window, false);
}


static void draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context){
  // get the row titles
  switch(cell_index->row){
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Very High",NULL,NULL );
    break;
    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "High",NULL,NULL );
    break;
    case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Normal",NULL,NULL );
    break;
    case 3:
      menu_cell_basic_draw(ctx, cell_layer, "Low",NULL,NULL );
    break;
    case 4:
      menu_cell_basic_draw(ctx, cell_layer, "Very Low",NULL,NULL );
    break;
  }
}

static void window_load(Window *window){

  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  GRect menu_layer_bounds;

  uint16_t offset = 2;
  uint16_t MENU_WIDTH = window_bounds.size.w - offset*2;

  // Create and add the question text
  uint16_t TITLE_HEIGHT = 33;
  s_title_text_layer = text_layer_create(
    GRect(0,0,window_bounds.size.w ,TITLE_HEIGHT));
  text_layer_set_text_alignment(s_title_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(s_title_text_layer, "Mood Today?");
  layer_add_child(window_layer,text_layer_get_layer(s_title_text_layer));


  menu_layer_bounds = GRect(offset,TITLE_HEIGHT-1,
                            MENU_WIDTH, window_bounds.size.h - TITLE_HEIGHT);

  // MAKE THE MENU LAYER
  // Make room for the large question at the top
  s_menu_layer = menu_layer_create(menu_layer_bounds);
  // using the second argument of menu_layer_set_callbacks, we can pass data
  // to each callback using the void pointer "callback_context".
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_cb,
    .draw_row = (MenuLayerDrawRowCallback) draw_row_cb,
    .get_cell_height = (MenuLayerGetCellHeightCallback) get_cell_height_cb,
    .select_click = (MenuLayerSelectCallback) select_click_cb
  });

  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  // set the default to the Middle position
  menu_layer_set_selected_index(s_menu_layer,
                                (MenuIndex){1,init_selection},
                                MenuRowAlignCenter, false);
  // add the menu child to the window
  layer_add_child(window_layer,menu_layer_get_layer(s_menu_layer));
  // see what the hell is going on with the heap amount
  APP_LOG(APP_LOG_LEVEL_ERROR, "heap size: used %d , free %d", heap_bytes_used(), heap_bytes_free());
}



static void window_unload(Window *window){
  // destroy the  menu layer
  menu_layer_destroy(s_menu_layer);
  text_layer_destroy(s_title_text_layer);
  window_destroy(window);
}


void pinteract_11(PinteractContext ctx){
  // roll the pstate over the appropriate number of days
  pi11_res.time_srt_priv_scrn = ctx.time_srt_priv_scrn;
  pi11_res.time_srt_pi = time(NULL); // time that we start the p interaction
  pi11_res.pinteract_code = 11;
  pi11_res.data_size = sizeof(Pinteract11Data);

  // initialize the local variables
  int8_t prev_state = get_pinteract_state_all().pi_11[1].mood_index;
  init_selection = (prev_state != -1 && prev_state<=4 && prev_state>=0) ? prev_state : 2;
  // create the window
  s_survey_window = window_create();
  window_set_window_handlers(s_survey_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });

  window_stack_push(s_survey_window,false);
}
