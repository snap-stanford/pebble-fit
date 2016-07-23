#pragma once

#include <pebble.h>

typedef void(CommJSReadyCallback)(void);

typedef enum {
  AppKeyJSReady,
  AppKeyStepsData,
  AppKeyStepsEndDate,
  AppKeyServerReceived,
} AppKey;

void comm_init();

void comm_deinit();