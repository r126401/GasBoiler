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




typedef enum EVENT_APP {

    EVENT_APP_SETPOINT_THRESHOLD,
    EVENT_APP_TIME_VALID,
    EVENT_APP_AUTO,
    EVENT_APP_MANUAL,
    EVENT_APP_ALARM_ON,
    EVENT_APP_ALARM_OFF,
    EVENT_APP_FACTORY,

}EVENT_APP;

typedef struct event_app_t {

    float value;
    EVENT_APP event_app;
} event_app_t;



#define DEFAULT_POWER  false
// Labels to params.
#define DEFAULT_TEMPERATURE 21.0
#define SETPOINT_TEMPERATURE "Umbral"
#define CALIBRATE "Calibrado"
#define READ_INTERVAL "intervalo de lectura"
#define MARGIN_TEMPERATURE "margen de temperatura"
#define ID_SENSOR "id_sensor"
#define MODE_THERMOSTAT "Modo"
#define NULSENSOR "Null"
#define ALARM  "alarm"
#define AUTO   "AUTO"
#define SELECT_MODE "SELECTMODE"
#define HEATING "Caldera"
#define SCHEDULE "Schedule"
#define STATS_HEATING "stats heating"

#define TEXT_STATUS_APP_FACTORY "SIN CONFIGURAR"
#define TEXT_STATUS_APP_ERROR "ERROR"
#define TEXT_STATUS_APP_AUTO "AUTO"
#define TEXT_STATUS_APP_MANUAL "MANUAL"
#define TEXT_STATUS_APP_STARTING "INICIALIZANDO"
#define TEXT_STATUS_APP_CONNECTING  "CONECTANDO"
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
   STATUS_APP_SYNCING,
   STATUS_APP_UPGRADING

} status_app_t;



void create_event_app_task();
//void send_event_app(event_app_t event);
void send_event_app_threshold(float threshold);
void send_event_app_alarm(EVENT_APP type);
void send_event_app_time_valid();
void send_event_app_status(EVENT_APP status);
void send_event_app_factory();

bool is_task_thermostat_active();
char* status2mnemonic(status_app_t status);
void set_app_update_threshold(float threshold, bool reporting);
float get_app_current_temperature();
float get_app_current_threshold();
void set_app_status(status_app_t status);
status_app_t get_app_status();
char* get_device_name();



