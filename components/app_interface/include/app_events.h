#include "stdbool.h"
#include "app_interface.h"


typedef enum EVENT_APP {

    EVENT_APP_SETPOINT_THRESHOLD,
    EVENT_APP_TIME_VALID,
    EVENT_APP_STATUS,
    EVENT_APP_ALARM_ON,
    EVENT_APP_ALARM_OFF,

}EVENT_APP;

typedef struct event_app_t {

    float value_float;
    int value_int;
    bool value_bool;
    EVENT_APP event_app;
} event_app_t;



void create_event_app_task();
void send_event_app_setpoint_temperature(float setpoint_temperature);
void send_event_app_status(status_app_t status) ;