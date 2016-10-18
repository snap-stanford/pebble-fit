#pragma once
#include <pebble.h>
#include "constants.h"
#include "helper.h"


static const uint16_t NUM_SEC_TRANSMIT_SERVER = 15;


void init_transmit_to_phone(enum TransmitReason trans_code);



// void init_transmit_all_to_phone();
// // this is triggered not by the wake up but by the worker.
// // the wakeups take care of getting everything back up and running
// // the worker deals with the other shit
// void init_transmit_all_to_server();
