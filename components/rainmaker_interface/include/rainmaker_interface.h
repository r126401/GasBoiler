#include "stdbool.h"
#include "app_interface.h"








void rainmaker_interface_init_environment();
float get_setpoint_temperature();
void notify_current_temperature(float current_temperature);
void notify_sensor_fail();
int get_read_interval();
status_app_t get_status_gas_boiler();
float get_current_temperature();
float get_temperature_correction();