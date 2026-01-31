


typedef enum EVENT_APP {

    EVENT_APP_SETPOINT_THRESHOLD,
    EVENT_APP_TIME_VALID,
    EVENT_APP_AUTO,
    EVENT_APP_MANUAL,
    EVENT_APP_CONNECTING,
    EVENT_APP_CONNECTED,
    EVENT_APP_SYNCING,
    EVENT_APP_SYNCED,
    EVENT_APP_ALARM_ON,
    EVENT_APP_ALARM_OFF,
    EVENT_APP_FACTORY,

}EVENT_APP;

typedef struct event_app_t {

    float value;
    EVENT_APP event_app;
} event_app_t;



void create_event_app_task();
void send_event_app_setpoint_temperature(float setpoint_temperature);
void send_event_app_status(EVENT_APP status);