#pragma once

#include <pebble.h>

typedef void(CommCallback)(void);
typedef void (DataWriteCallback)(DictionaryIterator*);

typedef enum {
  AppKeyJSReady,
  AppKeyStepsData,
  AppKeyStepsEndDate,
  AppKeyServerReceived,
} AppKey;

void comm_init();

void comm_deinit();

void comm_send_data(
  DataWriteCallback,
  AppMessageOutboxSent,
  AppMessageInboxReceived);