This file is for notes on general code structure and what elements to pull from different sources. 

Here are some pebble github examples that could work

https://github.com/pebble-examples/health-steps-remote
https://github.com/pebble-examples/health-watchface
https://github.com/pebble-examples/hrm-activity-example
https://github.com/pebble-examples/simple-analog-health
https://github.com/pebble/goodthink

Notes on Goodthink
Bugs 
	- communication.c : the vmc is uint16t, not uint8t

Good ideas
	- breaking down the health minute data into data fields that get sent over in different keys so that we can send the arrays over in separate fields in JSON uploads.

Bad ideas 
	- not using a time anchor to determine the last minute record to get
	- not having a limit on the number of bytes sent over. Simple to do, just set a limit on the data size and set the time anchor for the last minute record as the last record that was put into the dict. app_message_outbox_size_maximum(void) returns the size of the outbox that determines this. 
	- not having a fall back timer in case of watch being turned off

TODOS 
	-  