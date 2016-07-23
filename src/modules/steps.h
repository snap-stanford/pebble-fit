#pragma once

#include <pebble.h>
#include "comm.h"

void send_latest_steps_to_phone();

typedef enum {
  PersistKeyLastUploadTime = 0
} PersistKey;
