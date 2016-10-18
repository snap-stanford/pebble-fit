#include "display.h"

// structure of this code
// display the summary statistics for the past weeks
// 1. average MET
// 2. average|median sleep efficiency
// 3. average sleep length

// to do this, we need to
// 1. get the vmc data for each hour, get the total MET for that hour, then
// average across the day to get the summary MET for that day
// 2. get the sleep period data for each day, and then for each sleep period
// we get the minute level data and do Cole-Kripke, then use that to get sleep
// efficiency, then add that to the average for the day for sleep efficiency.
