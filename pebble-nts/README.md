### Wearable Psychiatry Pebble watch code outline

This readme is a basic plan for the structure of the pebble watch code, split
among the different functionalities it needs to have

1. Transmission of activity ("acti") data to server
  - minute level data sent up in 1 hour blocks
  - pinteract data
  - pebble health event data
2. Patient interaction ("pinteract")
  1. display specific surveys/tests at same time of day, each day
  2. pinteracts to implement
    - mood survey
    - sleep diary entry (quality, sleep duration)
    - sleep marker (press the button when going to sleep and waking up)  
  3. push the surveys up to the server when push the acti data
3.
  1. show recordings of previous surveys for limited set of surveys for past several days

Storage
  1. pinteract
    a. stored in persistent storage, where each pinteract key-value has a key ranging from 1 to the current value of the PINTERACT_KEY_COUNT_PERSIST_KEY. Thus, as we decrement the PINTERACT_KEY_COUNT_PERSIST_KEY as we send data to the server, the PINTERACT_KEY_COUNT_PERSIST_KEY points to the next key for the next pinteract in persistent storage. For this reason, all pinteract keys in the range 1 to the maximum value of PINTERACT_KEY_COUNT_PERSIST_KEY are off limits. Let us assume that PINTERACT_KEY_COUNT_PERSIST_KEY < 10000;
      - PINTERACT_KEY_COUNT_PERSIST_KEY: This key stores an integer counting the number of active keys in the PINTERACT_KEY_LIST_PERSIST_KEY data array. Also doubles as a pointer to the most recently stored pinteract data
  2. acti
    a. the data is stored in the Pebble firmware itself, but we need to keep track of which time periods have already been sent up to the server. Hence, we use a persistent storage key to keep track of the last time period that a block was sent up. Blocks are sent up in 1 hour blocks, so if we know the current time and the end time of the last block that was sent up, we can calculate what data remains to be sent up.
      - ACTI_LAST_UPLOAD_TIME_PERSIST_KEY : This key points to the end time of the last time period that was sent up

Scheduling
  1. In order for the pinteracts and the acti data to be shown and triggered to be sent up every day, we have to schedule several wakeup timers to go off
      1. some time of day when the pinteract needs to be shown to the user
      2. every hour when all data (acti, pinteract, and pebble health event) is attempted to be pushed to the phone and then server
      3. Backup wakeup times incase the watch dies, at 12hours in the future, at midnight, and 1 week in the future fallbacks
  2. The scheduling parameters are stored in two arrays.
    1. The first contains the pinteracts, and it does not change. This is an array of WakeupConfig structs with the pinteract code (to determine if the mood or sleep survey is to be asked) and the start and end times of the period in which they should appear. The index of the array is used to tie the specific wakeup timer to a given WakeupConfig element. To get around patient anticipation, the surveys are presented in a random time between the start and end times of the interval, though the interval itself is the same each day.
    2. The second array contains the wakeup ids themselves, with the index of the array corresponding one-to-one with the index of the WakeupConfig array, so that when a timer with a given wakeup id goes off we know which pinteract to start. This is made easier because we can specify a cookie to be stored with the wakeup id, so what when the wakeup timer goes off we get an integer associated with that wakeup. We simply set that integer to be the index corresponding to that wakeup id in the wakeup id array, and hence the same index corresponds to the pinteract that should be triggered in the WakeupConfig array.
    - NOTE, everytime a timer goes off all the events are rescheduled incase the watch was off for some period of time.
  3. The 1 hour timer transmission timer and 3 fallback timers are reset when any non-pinteract timer goes off.



NEED TO DO
  1. figure out how to configuration data back down from the server.


Pinteract design flow
- a privacy screen to popup before any surveys or tests, allow patient to dismiss the interaction for 30 minutes or 3 hours or to begin it.
- a data structure corresponding to each survey/test type, custom
- note, each data structure must have the pinteract code and size of the data struct at as the top 4 bytes (0:1 for pinteract code, 2:3 for data structure size). See /pinteract/pinteract_struct.h for an example. Note, we want a start time for when the privacy screen is presented, when the interaction is started, and when the interaction finishes.
