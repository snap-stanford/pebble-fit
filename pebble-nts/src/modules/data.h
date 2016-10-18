#pragma once
#include <pebble.h>
#include "helper.h"
#include "../constants.h"


bool data_to_send_acti();

bool data_to_send_pinteract();

int get_next_pinteract_element_key();

int get_data_size_of_pinteract_element(int pstorage_key);

int get_pinteract_code_of_pinteract_element(int pstorage_key);
