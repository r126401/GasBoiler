#include "stdbool.h"
#include "app_interface.h"


typedef enum EVENT_APP {
    EVENT_APP_WIFI,
    EVENT_APP_BROKER,
    EVENT_APP_OTA,
    EVENT_APP_SETPOINT_THRESHOLD,
    EVENT_APP_TIME_VALID,
    EVENT_APP_STATUS,
    EVENT_APP_ALARM_ON,
    EVENT_APP_ALARM_OFF,
    EVENT_APP_CALIBRATION,
    EVENT_APP_READ_INTERVAL,
    EVENT_APP_QR_DISPLAY,
    EVENT_APP_UPDATE_SCHEDULE,
    EVENT_APP_START_SCHEDULE,
    EVENT_APP_CHANGE_NAME

}EVENT_APP;

typedef struct event_app_t {

    float value_float;
    int value_int;
    bool value_bool;
    char* value_char;
    EVENT_APP event_app;
} event_app_t;



void create_event_app_task();
void send_event_app_setpoint_temperature(float setpoint_temperature);
void send_event_app_status(status_app_t status);
void send_event_app_calibration(float correction_temperature);
void send_event_app_read_interval(int read_interval);
void send_event_app_wifi_status(int status);
void send_event_app_broker_status(bool status);
void send_event_app_qr_display(char *qrcode);
void send_event_app_update_schedule();
void send_event_app_start_schedule(float setpoint_temperature);
void send_event_app_alarm(EVENT_APP alarm);
void send_event_app_ota_start();
void send_event_app_change_name(char* name);