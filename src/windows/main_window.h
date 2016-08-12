#pragma once

#include <pebble.h>

void main_window_push();

void main_window_update_steps(int step_count, int step_goal, int step_average);

void main_window_update_time(struct tm *tick_time);

void main_window_remove();