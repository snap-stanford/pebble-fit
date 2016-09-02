#include "launch.h"

#define USER_LAUNCH 0
#define WAKEUP_LAUNCH 1
#define OTHER_LAUNCH 2

static time_t s_time;
static int s_reason;

/* Add reason and date to out dict. */
static void launch_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyLaunchReason, &s_reason, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_time, sizeof(int), true);
}

/* Send launch event to phone. */
void launch_send_on_notification() {
  s_time = time(NULL);
  switch (launch_reason()) {
    case APP_LAUNCH_USER:
    case APP_LAUNCH_QUICK_LAUNCH:
      s_reason = USER_LAUNCH;
      break;
    case APP_LAUNCH_WAKEUP:
      s_reason = WAKEUP_LAUNCH;
      break;
    default:
      s_reason = OTHER_LAUNCH;
  }

  comm_send_data(launch_write, comm_sent_handler, comm_server_received_handler);
}

/* Add reason (placeholder) and date to out dict. */
static void delaunch_write(DictionaryIterator * out) {
  int placeholder = 0;
  dict_write_int(out, AppKeyDelaunchReason, &placeholder, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_time, sizeof(int), true);
}

/* Send delaunched event to phone. */
void launch_send_off_notification() {
  s_time = time(NULL);
  comm_send_data(delaunch_write, comm_sent_handler, NULL);
}