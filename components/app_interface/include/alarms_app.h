#include <stdio.h>
#include <stdbool.h>
#include "events_app.h"

#define N_ALARMS 4



#define ALARM_APP_OFF false
#define ALARM_APP_ON true


typedef enum ALARM_APP {

    ALL_ALARMS = -2,
    NO_ALARMS = -1,
    WIFI_ALARM = 0,
    MQTT_ALARM = 1,
    NTP_ALARM = 2,
    SENSOR_ALARM = 3
}ALARM_APP;

void init_alarms();

void set_alarm(ALARM_APP alarm, bool status);

void clear_all_alarms();

bool get_status_alarm(ALARM_APP alarm);

int get_active_alarms();

char* alarm2mnemonic(ALARM_APP alarm);




