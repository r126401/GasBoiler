#pragma once

#include <string.h>
#include "strings.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>


//#include <esp_log.h>
//#include <esp_event.h>
//#include <nvs_flash.h>
//#include <esp_sntp.h>
//#include <esp_timer.h>
//#include <esp_err.h>


//#include "esp_err.h"




#define DEFAULT_TEMPERATURE_CORRECTION -3.5
#define DEFAULT_ALARM "NO ALARM"
#define DEFAULT_READ_INTERVAL 60
#define INVALID_TEMPERATURE -100



#define TEXT_STATUS_APP_FACTORY "SIN CONFIGURAR"
#define TEXT_STATUS_APP_ERROR "ERROR"
#define TEXT_STATUS_APP_AUTO "AUTO"
#define TEXT_STATUS_APP_MANUAL "MANUAL"
#define TEXT_STATUS_APP_STARTING "INICIALIZANDO"
#define TEXT_STATUS_APP_SYNCHRONIZED "SINCRONIZADO"
#define TEXT_STATUS_APP_CONNECTING  "CONECTANDO"
#define TEXT_STATUS_APP_CONNECTED  "CONECTADO"
#define TEXT_STATUS_APP_SYNCING "SINCRONIZANDO"
#define TEXT_STATUS_APP_UPGRADING "ACTUALIZANDO"
#define TEXT_STATUS_APP_UNDEFINED "UNDEFINED"

#define EVT_RGB_TASK (1 << 0)
#define EVT_EVENT_TASK (1 << 1)
#define EVT_THERMOSTAK_TASK (1 << 2)
#define EVT_TUYA_TASK (1 << 3)


typedef enum status_app_t {

   STATUS_APP_FACTORY,
   STATUS_APP_ERROR,
   STATUS_APP_AUTO,
   STATUS_APP_MANUAL,
   STATUS_APP_STARTING,
   STATUS_APP_CONNECTING,
   STATUS_APP_CONNECTED,
   STATUS_APP_SYNCING,
   STATUS_APP_SYNCHRONIZED,
   STATUS_APP_UPGRADING,
   STATUS_APP_UNDEFINED

} status_app_t;



typedef enum {
    INDETERMINADO = -1,
    OFF = 0,
    ON = 1
} STATUS_RELAY;



void init_app_environment();
float get_setpoint_temperature();
void notify_current_temperature(float current_temperature);
void notify_sensor_fail();
int get_read_interval();
status_app_t get_status_gas_boiler();
float get_current_temperature();
float get_temperature_correction();
status_app_t get_current_status_app();
void reset_device();
void factory_reset_device();
void notify_setpoint_temperature(float setpoint_temperature);
void notify_heating_gas_Boiler(bool action) ;
void notify_status_factory(char* data_register);
void print_qr_register(char* register_data);
void set_status_app(status_app_t status);
void notify_device_started();
void notify_wifi_status(bool status);
void notify_mqtt_status(bool status);
void time_refresh(void *arg);
void update_time_valid(bool timevalid);
bool get_now(uint32_t *hour, uint32_t *min, uint32_t *sec);
STATUS_RELAY relay_operation(STATUS_RELAY op);
