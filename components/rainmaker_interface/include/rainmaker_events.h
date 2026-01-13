

#include <stdbool.h>

/**
 * @brief Make the actions to register device in cloud
 * 
 * @param register_data 
 */
void provisioning_device(char *register_data);

void notify_device_started();
void notify_wifi_status(bool status);
void notify_mqtt_status(bool status);

void time_refresh(void *arg);
void update_time_valid(bool timevalid);
void reset_device();
void factory_reset_device();