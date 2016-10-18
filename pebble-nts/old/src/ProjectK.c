/* Project Kraepelin, Main file
The MIT License (MIT)

Copyright (c) 2015, Nathaniel T. Stockham

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

This license is taken to apply to any other files in the Project Kraepelin
Pebble App roject.
*/


#include "ProjectK.h"



static void init() {
  /* ++++++ CHECK IF FIRST TIME APP INSTALLED +++++ */
  if((!persist_exists(I_BLK_PERSIST_KEY))
    || (!persist_exists(PK_VERSION_PERSIST_KEY))
    // || ( (persist_read_int(PK_VERSION_PERSIST_KEY) != CUR_PK_VERSION))
  ){

      // || (persist_read_int(PK_VERSION_PERSIST_KEY)< CUR_PK_VERSION)
    APP_LOG(APP_LOG_LEVEL_ERROR, "init PS");

    persist_write_int(PK_VERSION_PERSIST_KEY,CUR_PK_VERSION);
    init_persistent_storage();
  }
  /* ++++++ EVERY TIME THE APP STARTS  +++++ */
  // ALWAYS register the wakeup handler each time the app is opened so that,
  // when the app is closed again, that the wakeup handler will take place.
  wakeup_service_subscribe(wakeup_main_response_handler);

  // !! Register the global tick timer function. ALL elements will use this
  // and this tick timer alone!
  tick_timer_service_subscribe(SECOND_UNIT, fore_app_master_tick_timer_handler);

  /* ++++++ LAUNCH WORKER +++++ */
  // launch only if the app worker is not already running.
  // we use this section to do any initialization work
  // This is assuming that if the app background stopped due to
  // interference like the patient or being updated, that it is better to
  // start fresh.
  if(!app_worker_is_running()){
    // if the worker is NOT running,
    // TEMPORARY !!
    init_persistent_storage();

    APP_LOG(APP_LOG_LEVEL_ERROR, "launch worker");
    app_worker_launch();
  }

  /* ++++++ LAUNCH REASONS ++++++ */
  // APP_LOG(APP_LOG_LEVEL_ERROR, "made it to launch reason");

  // if the launch reason was due to the worker, then only send the messages and exit
  if(launch_reason() == APP_LAUNCH_WORKER){
    APP_LOG(APP_LOG_LEVEL_ERROR, "launched by worker");
    // since it doesn't hurt to call the config_wakeup_schedule()
    // over and over,
    config_wakeup_schedule();
    // EXECUTE logic based on foreground app wakeup reasons from worker
    worker_start_fore_app_reason_exec();
  }

  // if the launch was due to the user, then show the mood rating
  if(launch_reason() == APP_LAUNCH_USER){

    APP_LOG(APP_LOG_LEVEL_ERROR, "app start: heap size: used %d , free %d",
        heap_bytes_used(), heap_bytes_free());
    display_main_dash();
  }


  // if the launch was due to the wakeup, then show the mood rating.
  // NOTE, the wakeup handler deals with all the details of the wakeup
  if(launch_reason() ==  APP_LAUNCH_WAKEUP){
    // If the app was opened due to wakeup, then give the wakeup handler the
    // needed information to call open the correct pinteract
    WakeupId wakeup_id;
    int32_t wakeup_cookie;
    wakeup_get_launch_event(&wakeup_id, &wakeup_cookie);
    // APP_LOG(APP_LOG_LEVEL_ERROR, "wakeup_id %d  wakeup_cookie %d", (int) wakeup_id, (int) wakeup_cookie);
    wakeup_main_response_handler(wakeup_id, wakeup_cookie);
  }
}

static void deinit() {
  //   app_message_deregister_callbacks();
  tick_timer_service_unsubscribe();
  APP_LOG(APP_LOG_LEVEL_ERROR, "Main Project K rem heap B: %d",(int) heap_bytes_free());
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
