#pragma once

#include <pebble.h>

#define MAX_ENTRIES 60

int data_reload_steps(time_t * start, time_t * end);
int* data_get_steps_data();

typedef enum {
  PersistKeyLastUploadTime = 0
} PersistKey;
