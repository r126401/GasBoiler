

#include <inttypes.h>
#include <stdint.h>

uint8_t get_schedules_list();
uint8_t get_next_schedule(int *seconds_of_day, int *seconds_of_trigger, float *setpoint_temperature);