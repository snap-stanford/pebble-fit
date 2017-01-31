#pragma once

#include <pebble.h>

extern bool js_ready;

typedef void (CommCallback)(void);
typedef void (DataWriteCallback)(DictionaryIterator*);

/* Message keys (consistent with package.json). */
typedef enum {
  AppKeyJSReady,
  AppKeyDate,
  AppKeyServerReceived,
  AppKeyLaunchTime,
  AppKeyLaunchReason,
  AppKeyExitTime,
  AppKeyExitReason,
  AppKeyStorageKey,
  AppKeyStepsData,
  AppKeyStorageData,
  AppKeyArrayLength,
  AppKeyArrayStart,
  AppKeyConfigRequest,
  AppKeyTest0
} AppKey;

/* Init communication. */
void comm_init();

/* Handler for message sent. */
void comm_sent_handler(DictionaryIterator *iter, void *context);

/* Handler for getting message from server. */
void comm_server_received_handler(DictionaryIterator *iter, void *context);

/* Send data with data writing fn, and handlers. */
void comm_send_data(
  DataWriteCallback,
  AppMessageOutboxSent,
  AppMessageInboxReceived);
