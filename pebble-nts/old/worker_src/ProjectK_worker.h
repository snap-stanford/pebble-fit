#pragma once
#include <pebble_worker.h>

#include "constants_worker.h"
#include "helper_worker.h"
#include "fourier.h"
#include "raw_stats.h"
#include "acticlass_learn_alg.h"


static void write_blk_buf_to_persist();

static void summ_datalog();

static void reset_summ_metrics();

static void accel_data_handler(AccelData *data, uint32_t num_samples );

static void init_mem_log();

static void init();

static void deinit();

int main(void);
