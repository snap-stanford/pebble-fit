/**
 * This file was generated with Enamel : http://gregoiresage.github.io/enamel
 */

#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>
#include <pebble-events/pebble-events.h>
#include "enamel.h"

#ifndef ENAMEL_MAX_STRING_LENGTH
#define ENAMEL_MAX_STRING_LENGTH 100
#endif

#define ENAMEL_PKEY 3000000000
#define ENAMEL_DICT_PKEY (ENAMEL_PKEY+1)

typedef struct {
	EnamelSettingsReceivedHandler *handler;
	void *context;
} SettingsReceivedState;

static LinkedRoot *s_handler_list;

static EventHandle s_event_handle;

static DictionaryIterator s_dict;
static uint8_t* s_dict_buffer = NULL;
static uint32_t s_dict_size = 0;

static bool s_config_changed;

// -----------------------------------------------------
// Getter for 'is_consent'
bool enamel_get_is_consent(){
	Tuple* tuple = dict_find(&s_dict, 2703378645);
	return tuple ? tuple->value->int32 == 1 : false;
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'consent_name'
const char* enamel_get_consent_name(){
	Tuple* tuple = dict_find(&s_dict, 4163168280);
	return tuple ? tuple->value->cstring : "";
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'consent_email'
const char* enamel_get_consent_email(){
	Tuple* tuple = dict_find(&s_dict, 3428345362);
	return tuple ? tuple->value->cstring : "";
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_age'
const char* enamel_get_survey_age(){
	Tuple* tuple = dict_find(&s_dict, 3769805684);
	return tuple ? tuple->value->cstring : "";
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_gender'
const char* enamel_get_survey_gender(){
	Tuple* tuple = dict_find(&s_dict, 3415907915);
	return tuple ? tuple->value->cstring : "null";
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_height_unit'
const char* enamel_get_survey_height_unit(){
	Tuple* tuple = dict_find(&s_dict, 3564860411);
	return tuple ? tuple->value->cstring : "null";
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_height'
const char* enamel_get_survey_height(){
	Tuple* tuple = dict_find(&s_dict, 2106823441);
	return tuple ? tuple->value->cstring : "";
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_weight_unit'
const char* enamel_get_survey_weight_unit(){
	Tuple* tuple = dict_find(&s_dict, 3788605488);
	return tuple ? tuple->value->cstring : "null";
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_weight'
const char* enamel_get_survey_weight(){
	Tuple* tuple = dict_find(&s_dict, 17578608);
	return tuple ? tuple->value->cstring : "";
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'survey_race'
const char* enamel_get_survey_race(){
	Tuple* tuple = dict_find(&s_dict, 4017778921);
	return tuple ? tuple->value->cstring : "null";
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'optin'
bool enamel_get_optin(){
	Tuple* tuple = dict_find(&s_dict, 251311563);
	return tuple ? tuple->value->int32 == 1 : false;
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'vibrate'
bool enamel_get_vibrate(){
	Tuple* tuple = dict_find(&s_dict, 3086721838);
	return tuple ? tuple->value->int32 == 1 : false;
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'daily_start_time'
uint32_t enamel_get_daily_start_time(){
	Tuple* tuple = dict_find(&s_dict, 280095890);
	char* value =  tuple ? tuple->value->cstring : "09:00";
	uint32_t sec = atoi(value) * 3600 + atoi(value+3) * 60;
	if(strlen(value) > 6){
		sec += atoi(value+6);
	}
	return sec;
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'daily_end_time'
uint32_t enamel_get_daily_end_time(){
	Tuple* tuple = dict_find(&s_dict, 988182165);
	char* value =  tuple ? tuple->value->cstring : "20:00";
	uint32_t sec = atoi(value) * 3600 + atoi(value+3) * 60;
	if(strlen(value) > 6){
		sec += atoi(value+6);
	}
	return sec;
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'sleep_minutes'
SLEEP_MINUTESValue enamel_get_sleep_minutes(){
	Tuple* tuple = dict_find(&s_dict, 2891097914);
	return tuple ? atoi(tuple->value->cstring) : 60;
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'step_threshold'
STEP_THRESHOLDValue enamel_get_step_threshold(){
	Tuple* tuple = dict_find(&s_dict, 1001368360);
	return tuple ? atoi(tuple->value->cstring) : 500;
}
// -----------------------------------------------------

// -----------------------------------------------------
// Getter for 'display_duration'
DISPLAY_DURATIONValue enamel_get_display_duration(){
	Tuple* tuple = dict_find(&s_dict, 1482180045);
	return tuple ? atoi(tuple->value->cstring) : 10;
}
// -----------------------------------------------------


static uint16_t prv_get_inbound_size() {
	return 1
		+ 7 + 4
		+ 7 + ENAMEL_MAX_STRING_LENGTH
		+ 7 + ENAMEL_MAX_STRING_LENGTH
		+ 7 + ENAMEL_MAX_STRING_LENGTH
		+ 7 + 12
		+ 7 + 7
		+ 7 + ENAMEL_MAX_STRING_LENGTH
		+ 7 + 5
		+ 7 + ENAMEL_MAX_STRING_LENGTH
		+ 7 + 9
		+ 7 + 4
		+ 7 + 4
		+ 7 + ENAMEL_MAX_STRING_LENGTH
		+ 7 + ENAMEL_MAX_STRING_LENGTH
		+ 7 + 3
		+ 7 + 4
		+ 7 + 3
;
}

static uint32_t prv_map_messagekey(const uint32_t key){
	if( key == MESSAGE_KEY_is_consent) return 2703378645;
	if( key == MESSAGE_KEY_consent_name) return 4163168280;
	if( key == MESSAGE_KEY_consent_email) return 3428345362;
	if( key == MESSAGE_KEY_survey_age) return 3769805684;
	if( key == MESSAGE_KEY_survey_gender) return 3415907915;
	if( key == MESSAGE_KEY_survey_height_unit) return 3564860411;
	if( key == MESSAGE_KEY_survey_height) return 2106823441;
	if( key == MESSAGE_KEY_survey_weight_unit) return 3788605488;
	if( key == MESSAGE_KEY_survey_weight) return 17578608;
	if( key == MESSAGE_KEY_survey_race) return 4017778921;
	if( key == MESSAGE_KEY_optin) return 251311563;
	if( key == MESSAGE_KEY_vibrate) return 3086721838;
	if( key == MESSAGE_KEY_daily_start_time) return 280095890;
	if( key == MESSAGE_KEY_daily_end_time) return 988182165;
	if( key == MESSAGE_KEY_sleep_minutes) return 2891097914;
	if( key == MESSAGE_KEY_step_threshold) return 1001368360;
	if( key == MESSAGE_KEY_display_duration) return 1482180045;
	return 0;
}

static void prv_key_update_cb(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context){
}

static bool prv_each_settings_received(void *this, void *context) {
	SettingsReceivedState *state=(SettingsReceivedState *)this;
	state->handler(state->context);
	return true;
}

static void prv_inbox_received_handle(DictionaryIterator *iter, void *context) {
	if(dict_find(iter, MESSAGE_KEY_is_consent)){
		if(s_dict_buffer){
			free(s_dict_buffer);
			s_dict_buffer = NULL;
		}
		s_dict_size = dict_size(iter);
		s_dict_buffer = malloc(s_dict_size);

		Tuple *tuple=dict_read_first(iter);
		while(tuple){
			tuple->key = prv_map_messagekey(tuple->key);
			tuple=dict_read_next(iter);
		}

		dict_write_begin(&s_dict, s_dict_buffer, s_dict_size);
		dict_write_end(&s_dict);
		dict_merge(&s_dict, &s_dict_size, iter, false, prv_key_update_cb, NULL);

		if(s_handler_list){
			linked_list_foreach(s_handler_list, prv_each_settings_received, NULL);
		}

		s_config_changed = true;
	}
}

static uint16_t prv_save_generic_data(uint32_t startkey, const void *data, uint16_t size){
	uint16_t offset = 0;
	uint16_t total_w_bytes = 0;
	uint16_t w_bytes = 0;
	while(offset < size){
		w_bytes = size - offset < PERSIST_DATA_MAX_LENGTH ? size - offset : PERSIST_DATA_MAX_LENGTH;
		w_bytes = persist_write_data(startkey + offset / PERSIST_DATA_MAX_LENGTH, data + offset, w_bytes);
		total_w_bytes += w_bytes;
		offset += PERSIST_DATA_MAX_LENGTH;
	}
	return total_w_bytes; 
}

static uint16_t prv_load_generic_data(uint32_t startkey, void *data, uint16_t size){
	uint16_t offset = 0;
	uint16_t total_r_bytes = 0;
	uint16_t expected_r_bytes = 0;
	uint16_t r_bytes = 0;
	while(offset < size){
		if(size - offset > PERSIST_DATA_MAX_LENGTH){
			expected_r_bytes = PERSIST_DATA_MAX_LENGTH;
		}
		else {
			expected_r_bytes = size - offset;
		}
		r_bytes = persist_read_data(startkey + offset / PERSIST_DATA_MAX_LENGTH, data + offset, expected_r_bytes);
		total_r_bytes += r_bytes;
		if(r_bytes != expected_r_bytes){
			break; 
		}
		offset += PERSIST_DATA_MAX_LENGTH;
	}
	return total_r_bytes;
}

void enamel_init(){
	if(persist_exists(ENAMEL_PKEY) && persist_exists(ENAMEL_DICT_PKEY)) 
	{
		s_dict_size = persist_read_int(ENAMEL_PKEY);
		s_dict_buffer = malloc(s_dict_size);
		prv_load_generic_data(ENAMEL_DICT_PKEY, s_dict_buffer, s_dict_size);
	}
	else {
		s_dict_size = 0;
		s_dict_buffer = NULL;
	}

	dict_read_begin_from_buffer(&s_dict, s_dict_buffer, s_dict_size);
	
	s_config_changed = false;
	s_event_handle = events_app_message_register_inbox_received(prv_inbox_received_handle, NULL);
	events_app_message_request_inbox_size(prv_get_inbound_size());
}

void enamel_deinit(){
	if(s_config_changed){
		persist_write_int(ENAMEL_PKEY, s_dict_size);
		prv_save_generic_data(ENAMEL_DICT_PKEY, s_dict_buffer, s_dict_size);
	}

	if(s_dict_buffer){
		free(s_dict_buffer);
		s_dict_buffer = NULL;
	}

	s_config_changed = false;
	events_app_message_unsubscribe(s_event_handle);
}

EventHandle enamel_settings_received_subscribe(EnamelSettingsReceivedHandler *handler, void *context) {
	if (!s_handler_list) {
		s_handler_list = linked_list_create_root();
	}

	SettingsReceivedState *this = malloc(sizeof(SettingsReceivedState));
	this->handler = handler;
	this->context = context;
	linked_list_append(s_handler_list, this);

	return this;
}

void enamel_settings_received_unsubscribe(EventHandle handle) {
	int16_t index = linked_list_find(s_handler_list, handle);
	if (index == -1) {
		return;
	}

	free(linked_list_get(s_handler_list, index));
	linked_list_remove(s_handler_list, index);
	if (linked_list_count(s_handler_list) == 0) {
		free(s_handler_list);
		s_handler_list = NULL;
	}
}