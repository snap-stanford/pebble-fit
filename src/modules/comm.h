#pragma once

#include <pebble.h>

typedef void(CommCallback)(void);
typedef void (DataWriteCallback)(DictionaryIterator*);

typedef enum {
  AppKeyJSReady,
  AppKeyDate,
  AppKeyServerReceived,
  AppKeyLaunchReason,
  AppKeyDelaunchReason,
  AppKeyStorageKey,
  AppKeyStepsData,
  AppKeyStorageData,
  AppKeyArrayLength,
  AppKeyArrayStart
} AppKey;

void comm_init();

void comm_sent_handler(DictionaryIterator *iter, void *context);

void comm_server_received_handler(DictionaryIterator *iter, void *context);

void comm_deinit();

void comm_send_data(
  DataWriteCallback,
  AppMessageOutboxSent,
  AppMessageInboxReceived);