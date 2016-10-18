#include "pinteract_func.h"

// RAM MEMORY USAGE : 954 Bytes


static Window *s_survey_window;

static MenuLayer *s_menu_layer;
static TextLayer *s_title_text_layer;
static TextLayer *s_up_text_text_layer;
static TextLayer *s_down_text_text_layer;

static uint16_t NUM_ROWS;
static uint16_t ROW_HEIGHT;

// // possibly need to make these static
// uint16_t TITLE_HEIGHT;
// uint16_t MENU_WIDTH;
// uint16_t MENU_LEFT_PX;
// uint16_t OPTION_LEFT_PX;

static struct pif_survey_params* pif_srvy_prm_lcl;


static uint16_t get_num_rows_cb(MenuLayer *menu_layer, uint16_t section_index, void *callback_context){
  return NUM_ROWS;
}

// default number of sections is one

static uint16_t get_cell_height_cb(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
  return ROW_HEIGHT;
}

struct survey_opt_return optNfptr_exec(uint16_t opt_i, bool exec){
    struct survey_opt_return sopr;
    if(exec){
      #ifdef PBL_COLOR
        window_stack_remove(s_survey_window,true);
      #else
        window_stack_pop_all(false);
      #endif
    }

    switch(opt_i){
    case 0 :
    sopr = pif_srvy_prm_lcl->opt0fptr(exec);
    break;
    case 1 :
    sopr = pif_srvy_prm_lcl->opt1fptr(exec);
    break;
    case 2 :
    sopr = pif_srvy_prm_lcl->opt2fptr(exec);
    break;
    case 3 :
    sopr = pif_srvy_prm_lcl->opt3fptr(exec);
    break;
    case 4 :
    sopr = pif_srvy_prm_lcl->opt4fptr(exec);
    break;
    case 5 :
    sopr = pif_srvy_prm_lcl->opt5fptr(exec);
    break;
    case 6 :
    sopr = pif_srvy_prm_lcl->opt6fptr(exec);
    break;
    case 7 :
    sopr = pif_srvy_prm_lcl->opt7fptr(exec);
    break;
    case 8 :
    sopr = pif_srvy_prm_lcl->opt8fptr(exec);
    break;
  }

  return sopr;
}




// we do not use the section header because the font size is only 14

static void select_click_cb(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context){

  // close out the mood record menu entirely
  window_stack_remove(s_survey_window, false);
  psleep(100); // wait a bit for the OS to reset the window pointers

  // execute the option function
  optNfptr_exec(cell_index->row, true);
}


static void draw_row_cb(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context){
  //     struct survey_opt_return{
  //   char title[30];
  //   char subtitle[30];
  // };
  struct survey_opt_return sopr = optNfptr_exec(cell_index->row, false);

  if(pif_srvy_prm_lcl->row_subtitle_flag){
    menu_cell_basic_draw(ctx, cell_layer, sopr.title,sopr.subtitle,NULL );
  }else{
    menu_cell_basic_draw(ctx, cell_layer, sopr.title,NULL,NULL );
  }
}

static void window_load(Window *window){

  uint16_t TITLE_HEIGHT;
  uint16_t MENU_WIDTH;
  uint16_t MENU_LEFT_PX;
  uint16_t OPTION_LEFT_PX;

  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  //   Set the positions based on what type of survey it is
  //     GRect menu_layer_bounds, title_text_layer_bounds;
  //     bitmap_up_arrow_layer_bounds;

  GRect menu_layer_bounds;

  switch(pif_srvy_prm_lcl->survey_type){
    // this case is for the regular survery with words in the rows
    case 0 :

      ROW_HEIGHT = (pif_srvy_prm_lcl->row_height == 0) ? 26 : pif_srvy_prm_lcl->row_height;
      uint16_t offset = 2;
      MENU_WIDTH = window_bounds.size.w - offset*2;


      if(pif_srvy_prm_lcl->title_flag){
        // Create and add the question text
        TITLE_HEIGHT = 33;
        s_title_text_layer = text_layer_create(
          GRect(0,0,window_bounds.size.w ,TITLE_HEIGHT));
        text_layer_set_text_alignment(s_title_text_layer, GTextAlignmentCenter);
        text_layer_set_font(s_title_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
        text_layer_set_text(s_title_text_layer,pif_srvy_prm_lcl->title);
        layer_add_child(window_layer,text_layer_get_layer(s_title_text_layer));
      }else{
        TITLE_HEIGHT = 0;
      }

      menu_layer_bounds = GRect(offset,TITLE_HEIGHT-1,
                                MENU_WIDTH, window_bounds.size.h - TITLE_HEIGHT);
      break;
      // this is for Numeric surveys. Bipolar is two options, one at top and bottom
      // unipolar is a subclass where we just set the down option to " " blank
    case 1 :

      ROW_HEIGHT = (pif_srvy_prm_lcl->row_height == 0) ? 32 : pif_srvy_prm_lcl->row_height;
      MENU_WIDTH = 20;
      MENU_LEFT_PX = window_bounds.size.w - MENU_WIDTH ;
      OPTION_LEFT_PX = 2;

      if(pif_srvy_prm_lcl->title_flag){
        // Create and add the question text
        TITLE_HEIGHT = 75;
        s_title_text_layer = text_layer_create(
          GRect(5,50,MENU_LEFT_PX - 5 ,TITLE_HEIGHT));
        text_layer_set_text_alignment(s_title_text_layer, GTextAlignmentLeft);
        text_layer_set_font(s_title_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
        text_layer_set_text(s_title_text_layer,pif_srvy_prm_lcl->title);
        layer_add_child(window_layer,text_layer_get_layer(s_title_text_layer));
      }else{
        TITLE_HEIGHT = 0;
      }

      menu_layer_bounds = GRect(MENU_LEFT_PX,2, MENU_WIDTH, (168));

      // NOW, write the extra text layers
      // Create and add the up_option text
      s_up_text_text_layer = text_layer_create(
        GRect(OPTION_LEFT_PX,2, MENU_LEFT_PX-OPTION_LEFT_PX,ROW_HEIGHT+1));
      text_layer_set_text_alignment(s_up_text_text_layer, GTextAlignmentRight);
      text_layer_set_font(s_up_text_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
      text_layer_set_text(s_up_text_text_layer,pif_srvy_prm_lcl->up_text);
      text_layer_set_background_color(s_up_text_text_layer, GColorBlack);
      text_layer_set_text_color(s_up_text_text_layer, GColorWhite);
      layer_add_child(window_layer,text_layer_get_layer(s_up_text_text_layer));

      // Create and add the down_option text
      s_down_text_text_layer = text_layer_create(
        GRect(OPTION_LEFT_PX,168-ROW_HEIGHT-3,MENU_LEFT_PX-OPTION_LEFT_PX,ROW_HEIGHT+1));
      text_layer_set_text_alignment(s_down_text_text_layer, GTextAlignmentRight);
      text_layer_set_font(s_down_text_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
      text_layer_set_text(s_down_text_text_layer,pif_srvy_prm_lcl->down_text);
      text_layer_set_background_color(s_down_text_text_layer, GColorBlack);
      text_layer_set_text_color(s_down_text_text_layer, GColorWhite);
      layer_add_child(window_layer,text_layer_get_layer(s_down_text_text_layer));

      break;
    default :
      APP_LOG(APP_LOG_LEVEL_ERROR, "Warning, no such survey type");
      window_stack_remove(s_survey_window,false);
      psleep(100);
      break;
  }

  // MAKE THE MENU LAYER

  // Make room for the large question at the top
  s_menu_layer = menu_layer_create(menu_layer_bounds);
  // using the second argument of menu_layer_set_callbacks, we can pass data
  // to each callback using the void pointer "callback_context".
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_cb,
    .draw_row = (MenuLayerDrawRowCallback) draw_row_cb,
    .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_cb,
    .select_click = (MenuLayerSelectCallback)select_click_cb
  });

  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  // set the default to the Middle position
  menu_layer_set_selected_index(s_menu_layer,
                                (MenuIndex){1,pif_srvy_prm_lcl->init_selection},
                                MenuRowAlignCenter, false);
  // add the menu child to the window
  layer_add_child(window_layer,menu_layer_get_layer(s_menu_layer));
  // see what the hell is going on with the heap amount
  APP_LOG(APP_LOG_LEVEL_ERROR, "heap size: used %d , free %d", heap_bytes_used(), heap_bytes_free());

}



static void window_unload(Window *window){

  // destroy the  menu layer
  menu_layer_destroy(s_menu_layer);

  // remove the time text layer
  if(pif_srvy_prm_lcl->title_flag){
    text_layer_destroy(s_title_text_layer);
  }
  if(pif_srvy_prm_lcl->survey_type == 1){
    text_layer_destroy(s_up_text_text_layer);
    text_layer_destroy(s_down_text_text_layer);
  }

}


uint16_t pinteract_Nitem_survey(struct pif_survey_params* pif_srvy_prm){

  pif_srvy_prm_lcl = pif_srvy_prm;

  //   memcpy(pif_ll_el_lcl,pif_ll_el,sizeof(pif_ll_el)); DONT WORK CAUSE POINTERS< NOT REAL YET

  NUM_ROWS = pif_srvy_prm_lcl->nitems;

  s_survey_window = window_create();
  window_set_window_handlers(s_survey_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });
  // cause this function is not defined in basalt, cause it doesn't have status bar
  #ifdef PBL_SDK_2
  window_set_fullscreen(s_survey_window,true);
  #endif

  window_stack_push(s_survey_window,false);

  return 100;
}
