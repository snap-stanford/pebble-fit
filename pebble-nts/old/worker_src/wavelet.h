#pragma once
#include <pebble_worker.h>
#include "helper_worker.h"

#include "constants_worker.h"

// be sure to use non-static functions, so that they can be seen by the main
// worker file


void ordering_switch(int16_t *d, int16_t lb, int16_t ub, int16_t ofs);

void decomp_haar_order(int16_t *d, int16_t lb, int16_t ub);

void recon_haar_order(int16_t *d, int16_t lb, int16_t ub);


void haar_add_sub_adjacent(int16_t *d, int16_t i_ubnd, int32_t mul, int32_t div );

// the haar wavelet decomposition
void dwt_haar(int16_t *d, int16_t dlenpwr, int16_t depth);

// the haar wavelet reconstruction
void idwt_haar(int16_t *d, int16_t dlenpwr, int16_t depth);
