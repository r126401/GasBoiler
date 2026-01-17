#include "stdbool.h"
#include "app_interface.h"
#include "esp_err.h"







void rainmaker_interface_init_environment();
float platform_get_setpoint_temperature();
void platform_notify_current_temperature(float current_temperature);
void platform_notify_sensor_fail();
int platform_get_read_interval();
status_app_t platform_get_status_gas_boiler();
float platform_get_current_temperature();
float platform_get_temperature_correction();
void platform_reset_device();
void platform_factory_reset_device();
esp_err_t platform_notify_setpoint_temperature(float setpoint_temperature);
esp_err_t platform_notify_heating_gas_Boiler(bool action);
char* platform_get_device_name();