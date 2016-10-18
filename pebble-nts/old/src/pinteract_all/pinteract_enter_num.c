#include "pinteract_func.h"

static Window *s_enter_num_window;

static TextLayer *s_row_title_text_layer_ary[2];

static TextLayer *s_num_text_layer_ary[4];
static TextLayer *s_label_text_layer_ary[4];

static uint8_t cur_opt;
static int32_t cur_num[4];
static char num_buf[4][8];
static char label_buf[4][8];



static struct pif_enter_num_params* prm_lcl;

static void select_click_handler(ClickRecognizerRef recognizer, void *context){
  // invert the current opt from black background white text to
  text_layer_set_background_color(s_num_text_layer_ary[cur_opt], GColorWhite);
  text_layer_set_text_color(s_num_text_layer_ary[cur_opt], GColorBlack);

  cur_opt = (prm_lcl->opt_fptr_ary[cur_opt](NULL)).next_opt_i; // move to the next opt

  // if have reached the final opt, then give the data to the finish function
  if(cur_opt == 4){
    window_stack_remove(s_enter_num_window, false);
    psleep(100); // wait a bit for the OS to reset the window pointers
    (prm_lcl->opt_fptr_ary[3])(cur_num); // this is the final finishing function
  }else{
    // invert the current opt from white background black text
    text_layer_set_background_color(s_num_text_layer_ary[cur_opt], GColorBlack);
    text_layer_set_text_color(s_num_text_layer_ary[cur_opt], GColorWhite);
  }
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context){
  // invert the current opt from black background white text to
  text_layer_set_background_color(s_num_text_layer_ary[cur_opt], GColorWhite);
  text_layer_set_text_color(s_num_text_layer_ary[cur_opt], GColorBlack);

  cur_opt = (prm_lcl->opt_fptr_ary[cur_opt](NULL)).prev_opt_i; // move to previous opt

  // invert the current opt from white background black text
  text_layer_set_background_color(s_num_text_layer_ary[cur_opt], GColorBlack);
  text_layer_set_text_color(s_num_text_layer_ary[cur_opt], GColorWhite);

}

static void up_click_handler(ClickRecognizerRef recognizer, void *context){
  // increment the number until reach higher limit
  int32_t high_num_limit = (prm_lcl->opt_fptr_ary[cur_opt](NULL)).num_max;
  int32_t num_delta =   (prm_lcl->opt_fptr_ary[cur_opt](NULL)).num_delta;
  cur_num[cur_opt] =
    (cur_num[cur_opt] < high_num_limit) ? (cur_num[cur_opt] + num_delta) : high_num_limit;
  // update the text layer
  snprintf(num_buf[cur_opt], sizeof(num_buf[cur_opt]), "%d",(int)cur_num[cur_opt]);
  text_layer_set_text(s_num_text_layer_ary[cur_opt], num_buf[cur_opt]);
  layer_mark_dirty((Layer*) s_num_text_layer_ary[cur_opt]);
}


static void down_click_handler(ClickRecognizerRef recognizer, void *context){
  // increment the number until reach higher limit
  int32_t low_num_limit = (prm_lcl->opt_fptr_ary[cur_opt](NULL)).num_min;
  int32_t num_delta =   (prm_lcl->opt_fptr_ary[cur_opt](NULL)).num_delta;
  cur_num[cur_opt] =
    (cur_num[cur_opt] > low_num_limit) ? (cur_num[cur_opt] - num_delta) : low_num_limit;
  // update the text layer
  snprintf(num_buf[cur_opt], sizeof(num_buf[cur_opt]), "%d",(int)cur_num[cur_opt]);
  text_layer_set_text(s_num_text_layer_ary[cur_opt], num_buf[cur_opt]);
  layer_mark_dirty((Layer*) s_num_text_layer_ary[cur_opt]);

}

static void click_config_provider(void *context){
  // single clicks, keep it simple
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}



void enter_num_window_load(Window *window) {
  Layer* window_layer = window_get_root_layer(window);
  // GRect window_bounds = layer_get_bounds(window_layer);
  window_set_background_color(window,GColorWhite);

  GRect out_of_frame = GRect(0,0,0,0);

  // define the relative positions of all the elements
  // dimensions that vary with # of rows
  uint8_t rows_x_ofs = 13;
  uint8_t rows_title_w = 120;
  uint8_t num_label_sep = 2;

  // for 1 row
  uint8_t rows1_y_ofs = 9;
  uint8_t rows1_title_h = 65;
  char * rows1_title_font = FONT_KEY_GOTHIC_28_BOLD;
  uint8_t rows1_num_h = 33;
  char * rows1_num_font = FONT_KEY_GOTHIC_24_BOLD;
  uint8_t rows1_label_h = 27;
  char * rows1_label_font = FONT_KEY_GOTHIC_18;
  // for 2 rows
  uint8_t rows2_y_ofs = 6;
  uint8_t rows2_title_h = 32;
  char * rows2_title_font = FONT_KEY_GOTHIC_24_BOLD;
  uint8_t rows2_num_h = 33;
  char * rows2_num_font = FONT_KEY_GOTHIC_24;
  uint8_t rows2_label_h = 27;
  char * rows2_label_font = FONT_KEY_GOTHIC_18;
  uint8_t rows_sep = rows2_title_h + rows2_num_h + 5;

  // for 1 col
  uint8_t cols1_x_ofs = 10;
  uint8_t cols1_num_w = 75;
  uint8_t cols1_label_w = 40;
  // for 2 col
  int8_t cols2_x_ofs = -3;
  uint8_t cols2_num_w = 33;
  uint8_t cols2_label_w = 24;
  uint8_t cols_sep = cols2_num_w + num_label_sep + cols2_label_w + 9;


  // static TextLayer *s_row_title_text_layer_ary[2];
  // static TextLayer *s_num_text_layer_ary[4];
  // static TextLayer *s_label_text_layer_ary[4];

  // this is just for creating and some slight modification. adding layers,
  // adding text, comes below
  if(prm_lcl->n_rows == 1){
    // create the title layers
    s_row_title_text_layer_ary[0] =
      text_layer_create(GRect(rows_x_ofs,rows1_y_ofs,rows_title_w,rows1_title_h));
    text_layer_set_font(s_row_title_text_layer_ary[0],fonts_get_system_font(rows1_title_font));


    s_row_title_text_layer_ary[1] = text_layer_create(out_of_frame);

    // ++++++++++++++++ROW 0++++++++++++++++
    // COLS 1
    if(prm_lcl->n_cols_r0 == 1){
      s_num_text_layer_ary[0] =
        text_layer_create(GRect(
          rows_x_ofs + cols1_x_ofs,
          rows1_y_ofs + rows1_title_h,
          cols1_num_w, rows1_num_h));
      text_layer_set_font(s_num_text_layer_ary[0],fonts_get_system_font(rows1_num_font));

      s_label_text_layer_ary[0] =
        text_layer_create(GRect(
          rows_x_ofs + cols1_x_ofs + cols1_num_w + num_label_sep,
          rows1_y_ofs + rows1_title_h + (rows1_num_h - rows1_label_h),
          cols1_label_w, rows1_label_h));
      text_layer_set_font(s_label_text_layer_ary[0],fonts_get_system_font(rows1_label_font));

      s_num_text_layer_ary[1] = text_layer_create(out_of_frame);
      s_label_text_layer_ary[1] = text_layer_create(out_of_frame);
    // COLS 2
    }else if(prm_lcl->n_cols_r0 == 2){
      s_num_text_layer_ary[0] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs,
          rows1_y_ofs + rows1_title_h,
          cols2_num_w, rows1_num_h));
      text_layer_set_font(s_num_text_layer_ary[0],fonts_get_system_font(rows1_num_font));

      s_label_text_layer_ary[0] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs + cols2_num_w + num_label_sep,
          rows1_y_ofs + rows1_title_h + (rows1_num_h - rows1_label_h),
          cols2_label_w, rows1_label_h));
      text_layer_set_font(s_label_text_layer_ary[0],fonts_get_system_font(rows1_label_font));

      s_num_text_layer_ary[1] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs + cols_sep,
          rows1_y_ofs + rows1_title_h,
          cols2_num_w, rows1_num_h));
      text_layer_set_font(s_num_text_layer_ary[1],fonts_get_system_font(rows1_num_font));

      s_label_text_layer_ary[1] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs + cols_sep + cols2_num_w + num_label_sep ,
          rows1_y_ofs + rows1_title_h + (rows1_num_h - rows1_label_h),
          cols2_label_w, rows1_label_h));
      text_layer_set_font(s_label_text_layer_ary[1],fonts_get_system_font(rows1_label_font));
    }
    // got ahead and initialize the remaining nums/labels for row 2

    s_num_text_layer_ary[2] = text_layer_create(out_of_frame);
    s_label_text_layer_ary[2] = text_layer_create(out_of_frame);
    s_num_text_layer_ary[3] = text_layer_create(out_of_frame);
    s_label_text_layer_ary[3] = text_layer_create(out_of_frame);


  }else if(prm_lcl->n_rows == 2) {
    // title
    s_row_title_text_layer_ary[0] =
      text_layer_create(GRect(rows_x_ofs,rows2_y_ofs,rows_title_w,rows2_title_h));
    text_layer_set_font(s_row_title_text_layer_ary[0],fonts_get_system_font(rows2_title_font));
    s_row_title_text_layer_ary[1] =
      text_layer_create(GRect(rows_x_ofs,rows2_y_ofs + rows_sep,rows_title_w,rows2_title_h));
    text_layer_set_font(s_row_title_text_layer_ary[1],fonts_get_system_font(rows2_title_font));

    // ++++++++++++++++ROW 0++++++++++++++++
    // COLS 1
    if(prm_lcl->n_cols_r0 == 1){
      s_num_text_layer_ary[0] =
        text_layer_create(GRect(
          rows_x_ofs + cols1_x_ofs,
          rows2_y_ofs + rows2_title_h,
          cols1_num_w, rows2_num_h));
      text_layer_set_font(s_num_text_layer_ary[0],fonts_get_system_font(rows2_num_font));

      s_label_text_layer_ary[0] =
        text_layer_create(GRect(
          rows_x_ofs + cols1_x_ofs + cols1_num_w + num_label_sep,
          rows2_y_ofs + rows2_title_h + (rows2_num_h - rows2_label_h),
          cols1_label_w, rows2_label_h));
      text_layer_set_font(s_label_text_layer_ary[0],fonts_get_system_font(rows2_label_font));

      s_num_text_layer_ary[1] = text_layer_create(out_of_frame);
      s_label_text_layer_ary[1] = text_layer_create(out_of_frame);

    // COLS 2
    }else if(prm_lcl->n_cols_r0 == 2) {
      s_num_text_layer_ary[0] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs,
          rows2_y_ofs + rows2_title_h,
          cols2_num_w, rows2_num_h));
      text_layer_set_font(s_num_text_layer_ary[0],fonts_get_system_font(rows2_num_font));

      s_label_text_layer_ary[0] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs + cols2_num_w + num_label_sep,
          rows2_y_ofs + rows2_title_h + (rows2_num_h - rows2_label_h),
          cols2_label_w, rows2_label_h));
      text_layer_set_font(s_label_text_layer_ary[0],fonts_get_system_font(rows2_label_font));

      s_num_text_layer_ary[1] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs + cols_sep,
          rows2_y_ofs + rows2_title_h,
          cols2_num_w, rows2_num_h));
      text_layer_set_font(s_num_text_layer_ary[1],fonts_get_system_font(rows2_num_font));

      s_label_text_layer_ary[1] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs + cols_sep + cols2_num_w + num_label_sep,
          rows2_y_ofs + rows2_title_h + (rows2_num_h - rows2_label_h),
          cols2_label_w, rows2_label_h));
      text_layer_set_font(s_label_text_layer_ary[1],fonts_get_system_font(rows2_label_font));

    }

    // ++++++++++++++++ROW 1++++++++++++++++
    // COLS 1
    if(prm_lcl->n_cols_r1 == 1){
      s_num_text_layer_ary[2] =
        text_layer_create(GRect(
          rows_x_ofs + cols1_x_ofs,
          rows2_y_ofs + rows_sep + rows2_title_h,
          cols1_num_w, rows2_num_h));
      text_layer_set_font(s_num_text_layer_ary[2],fonts_get_system_font(rows2_num_font));

      s_label_text_layer_ary[2] =
        text_layer_create(GRect(
          rows_x_ofs + cols1_x_ofs + cols1_num_w + num_label_sep,
          rows2_y_ofs + rows_sep + rows2_title_h + (rows2_num_h - rows2_label_h),
          cols1_label_w, rows2_label_h));
      text_layer_set_font(s_label_text_layer_ary[2],fonts_get_system_font(rows2_label_font));

      s_num_text_layer_ary[3] = text_layer_create(out_of_frame);
      s_label_text_layer_ary[3] = text_layer_create(out_of_frame);

    // COLS 2
    }else if(prm_lcl->n_cols_r1 == 2) {
      s_num_text_layer_ary[2] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs,
          rows2_y_ofs + rows_sep + rows2_title_h,
          cols2_num_w, rows2_num_h));
      text_layer_set_font(s_num_text_layer_ary[2],fonts_get_system_font(rows2_num_font));

      s_label_text_layer_ary[2] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs + cols2_num_w + num_label_sep,
          rows2_y_ofs + rows_sep + rows2_title_h + (rows2_num_h - rows2_label_h),
          cols2_label_w, rows2_label_h));
      text_layer_set_font(s_label_text_layer_ary[2],fonts_get_system_font(rows2_label_font));

      s_num_text_layer_ary[3] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs + cols_sep,
          rows2_y_ofs + rows_sep + rows2_title_h,
          cols2_num_w, rows2_num_h));
      text_layer_set_font(s_num_text_layer_ary[3],fonts_get_system_font(rows2_num_font));

      s_label_text_layer_ary[3] =
        text_layer_create(GRect(
          rows_x_ofs + cols2_x_ofs + cols_sep + cols2_num_w + num_label_sep,
          rows2_y_ofs + rows_sep + rows2_title_h + (rows2_num_h - rows2_label_h),
          cols2_label_w, rows2_label_h));
      text_layer_set_font(s_label_text_layer_ary[3],fonts_get_system_font(rows2_label_font));

    }
  }



  // Once the bounds are set, then create the textlayers
  // create the title layers
  // static TextLayer *s_title_text_layer_ary[2];

  for(int16_t i = 0; i < 2; i++){
    text_layer_set_text(s_row_title_text_layer_ary[i],(prm_lcl->row_titles[i]));
    text_layer_set_text_alignment(s_row_title_text_layer_ary[i], GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_row_title_text_layer_ary[i]));
  }
  //
  // static TextLayer *s_num_text_layer_ary[4];
  // static TextLayer *s_label_text_layer_ary[4];
  for(int16_t i = 0; i < 4; i++){
    cur_num[i] = (prm_lcl->opt_fptr_ary[i](NULL)).num_init;
    snprintf(num_buf[i], sizeof(num_buf[i]), "%d",(int)cur_num[i]);
    text_layer_set_text(s_num_text_layer_ary[i], num_buf[i]);
    text_layer_set_text_alignment(s_num_text_layer_ary[i], GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_num_text_layer_ary[i]));

    strcpy(label_buf[i], (prm_lcl->opt_fptr_ary[i](NULL)).label);
    text_layer_set_text(s_label_text_layer_ary[i], label_buf[i]);
    text_layer_set_text_alignment(s_label_text_layer_ary[i], GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_label_text_layer_ary[i]));
  }
  //
  // // initialize the first selection
  text_layer_set_background_color(s_num_text_layer_ary[cur_opt], GColorBlack);
  text_layer_set_text_color(s_num_text_layer_ary[cur_opt], GColorWhite);

}


void enter_num_window_unload(Window *window) {
  // well, we create all the text layers, just the ones we dont' use are pushed
  // off to the side
  for(int16_t i = 0; i < 2; i++){
    text_layer_destroy(s_row_title_text_layer_ary[i]);
  }

  for(int16_t i = 0; i < 4; i++){
    text_layer_destroy(s_num_text_layer_ary[i]);
    text_layer_destroy(s_label_text_layer_ary[i]);
  }

}


uint16_t pinteract_enter_num(struct pif_enter_num_params* pif_enter_num_prm ){

  prm_lcl = pif_enter_num_prm;
  // we ALWAYS start with the first function
  cur_opt = 0;

  // push the blank transmit phone window to the front.
  s_enter_num_window = window_create();
  window_set_window_handlers(s_enter_num_window, (WindowHandlers) {
    .load = enter_num_window_load,
    .unload = enter_num_window_unload,
  });

  #ifdef PBL_SDK_2
    window_set_fullscreen(s_enter_num_window,true);
  #endif

  window_set_click_config_provider(s_enter_num_window,
                                   (ClickConfigProvider) click_config_provider);
  window_stack_push(s_enter_num_window, false);

  return 101;
}
