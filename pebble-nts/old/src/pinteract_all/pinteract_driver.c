#include "pinteract_func.h"



// use a function pointer, then a swtich case with the
// argument being an integer that is from the persistant storage
// that is set remotely (or, locally, eh) as a configuration


// NOTE ON LEXICAL SCOPE.
// These pointers must declared here because otherwise, when the
// pinteract_driver function terminates, all the pointers that were
// declared locally will go out of scope, so when they are dereferenced
// further down the chain they will cause all hell to break loose. Which,
// however, is a VERY important lesson when we are adding elements to the
// linked list of function calls. EACH interaction that adds an element
// to the linked list MUST declare the structs that they add as a GLOBAL
// variable, at least within that .c file, so that when the interactions'
// functions close out their pointers are not destroyed. NOTE!!, the
// pointers CAN be declared static so that they are ONLY viewable within
// that specific .c file, so their wont be any cluttering up of the name
// space to just call all the functions added to the linked list in a given
// interaction, like: added_pnteract_0, added_pnteract_1, etc.

static uint8_t res_buf[PERSIST_DATA_MAX_LENGTH] ={0};

// go ahead an initalize these structures to const, because IF we
// want to write to persistant storage (which we can NOT do if we
// change the .next ) member when we are assigning values to the
// sequence start element, we have to always terminate it like this.
static struct pinteract_func_ll_el res_buf_end = {
  .pif = NULL, .buf = res_buf, .next = NULL};
// static struct pinteract_func_ll_el write_to_ps = {
//     .pif = pinteract_alert, .buf = res_buf, .next = &res_buf_end};
static struct pinteract_func_ll_el write_to_ps = {
  .pif = pinteract_write_res_buf_to_pesistant_storage,
  .buf = res_buf, .next = &res_buf_end};
static struct pinteract_func_ll_el pinteract_seq_srt;

PinteractFunc pif_srt;




// typedef uint16_t PinteractCodeFunc( uint8_t *buf_res );

// This is the only function that REALLY needs to be public. It is very small
// just very simple functionality at this point that might expand if needed
void pinteract_driver(uint16_t pinteract_code ){

  if(pinteract_code > 0){
    pif_srt = pinteract_code_to_func_ptr(pinteract_code);
    // We start the byte counter with 2 to say 2 byte has been written
    write_res_buf_byte_count(res_buf, 0, sizeof(MAX_PINTERACT_PS_B_SIZE));
    // update the current active pinteract

    pinteract_seq_srt.pif = pif_srt;
    pinteract_seq_srt.buf = res_buf;
    pinteract_seq_srt.next = &write_to_ps;

    pinteract_priv_scrn(&pinteract_seq_srt);
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR,
            "WARNING, pinteract_code <= 0, no corresponding pinteracts");
  }
}


// CONVIENENCE
// the switch function that takes the desired pinteract
// code and returns with a PinteractCodeFunc pointer to that function
// this could be a public function, because other places might like such
// a use for debugging or quick and dirty displays
PinteractFunc pinteract_code_to_func_ptr(int16_t pinteract_code){

  PinteractFunc pif = pinteract_alert; // incase nothing matches, show alert screen

  switch(pinteract_code){

    case 11 :
      pif = pinteract_11;
      break;
    // case 12 :
    //   pif = pinteract_12;
    //   break;
    // case 13 :
    //   pif = pinteract_13;
    //   break;
    case 14 :
      pif = pinteract_14;
      break;
    // case 15:
    //   pif = pinteract_15;
    //   break;

    default :
    // APP_LOG
    // picf == some function that displays an error screen
    // or tells the configurer that that interaction does not exist.
    break;
  }
  return pif;
}
