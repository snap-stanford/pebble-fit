#include "pinteract.h"

// this i



// this function is called from the privacy screen
void pinteract_driver(int16_t pinteract_code, PinteractContext ctx){
  // move the entry history up by the appropriate number of days
  switch(pinteract_code){

    case 11 :
      pinteract_11(ctx);
      break;
    case 14 :
      pinteract_14(ctx);
      break;
    default :
    // APP_LOG
    // picf == some function that displays an error screen
    // or tells the configurer that that interaction does not exist.
    break;
  }

}
