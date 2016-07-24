#include "launch.h"

#define USER_LAUNCH 0
#define WAKEUP_LAUNCH 1
#define OTHER_LAUNCH 2

static time_t s_time;
static int s_reason;

static void data_write(DictionaryIterator * out) {
  dict_write_int(out, AppKeyLaunchReason, &s_reason, sizeof(int), true);
  dict_write_int(out, AppKeyDate, &s_time, sizeof(int), true);
}

void send_wakeup_reason() {
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

  comm_send_data(data_write, comm_sent_handler, comm_server_received_handler);
}