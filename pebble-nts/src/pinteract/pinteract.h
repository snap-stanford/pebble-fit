#pragma once

#include <pebble.h>
#include "../config_all/config_func.h"
#include "pinteract_structs.h"
#include "../modules/helper.h"

// STRUCTURE OF THE PINTERACT CONTORL FLOW
// 1. call start screen
//  a. privacy screen
//  b. pinteract driver
// 2. call first pinteract
//  a. pinteract driver
//  b. pinteract 11, 12, etc
// 3. capture response in persistant storage
// 4. close out of pinteract, either return to home screen or close enitrely
//  -> note, since we are launching a new screen, we can just close out and
//   get back to the home screen


typedef struct {
  time_t time_srt_priv_scrn;
}PinteractContext;

// note, we pass the privacy screen the pinteact_code that we want it to execute
// when the user agrees to begin
void pinteract_priv_scrn(int16_t pinteract_code);

// note, we decouple the privacy screen and pinteract driver because not all
// printeracts will require a privacy screen, but they will all need a driver.
void pinteract_driver(int16_t pinteract_code, PinteractContext ctx);

void pinteract_11(PinteractContext ctx);

void pinteract_14(PinteractContext ctx);
