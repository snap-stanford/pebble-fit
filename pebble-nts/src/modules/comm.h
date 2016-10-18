//  This file takes care of the transmission of data to the phone and pushing
// the data to the server.
# pragma once

#include <pebble.h>
#include "helper.h"
#include "data.h"
#include "../constants.h"

// this sends the specified data items, of three types.
// 1. send all hourly data
// 2. send all pinteract data
// 3. send signal to send to server

typedef enum{
  AppKeyJSReady = 0,
  AppKeyActiData = 1,
  AppKeyHealthEventData = 2,
  AppKeyConfigData = 3,
  AppKeyPinteractData = 4,
  AppKeyPushToServer = 5,
  AppKeySentToServer = 6,
  NumAppKeys = 7
} AppKey;


// ONLY NEEDED FOR TRANMISSION
typedef struct {
  uint8_t steps;
  uint8_t orientation;
  uint16_t vmc;
  uint8_t is_invalid;
  uint8_t light;
}__attribute__((__packed__)) TrunHealthMinuteData;

typedef struct{
  int16_t health_activity;
  time_t time_start;
  time_t time_end;
}__attribute__((__packed__)) HealthEventData;

void comm_begin_upload_countdown();

void comm_begin_upload_no_countdown();
