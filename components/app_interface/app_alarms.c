

#include "app_alarms.h"
#include "esp_log.h"
#include "app_events.h"

static char* TAG = "alarmas.c";


static bool alarms[N_ALARMS];

char* alarm2mnemonic(ALARM_APP alarm) {

    static char mnemonic[13] = {0};

    switch (alarm) {

        case ALL_ALARMS:
            strncpy(mnemonic,"ALL_ALARMS", 13 );
        break;
        case NO_ALARMS:
            strncpy(mnemonic,"NO_ALARMS", 13 );
        break;
        case WIFI_ALARM:
            strncpy(mnemonic,"WIFI_ALARM", 13 );
        break;
        case MQTT_ALARM:
            strncpy(mnemonic,"MQTT_ALARM", 13 );
        break;
        case NTP_ALARM:
            strncpy(mnemonic,"NTP_ALARM", 13 );
        break;
        case SENSOR_ALARM:
            strncpy(mnemonic,"SENSOR_ALARM", 13 );
        break;

    }
    return mnemonic;
}

void init_alarms() {

    int i;
    // Todas las alarmas a true. Se iran apagando segun 
    for (i=0;i<N_ALARMS;i++) {
        alarms[i] = true;
    }

}


void set_alarm(ALARM_APP alarm, bool status) {

    if (alarms[alarm] == status) {

        return;
    }
    alarms[alarm] = status;
    if (status == ALARM_APP_ON) {
        send_event_app_alarm(EVENT_APP_ALARM_ON);
    }

    if (get_active_alarms() == 0) {
        send_event_app_alarm(EVENT_APP_ALARM_OFF);
    }

}

void clear_all_alarms() {

}

bool get_status_alarm(ALARM_APP alarm) {

    return alarms[alarm];
}

int get_active_alarms() {

    int nalarms = 0;
    int i;
    for (i=0;i<N_ALARMS;i++) {

        if (alarms[i] == true) nalarms++;

    }

    ESP_LOGI(TAG, "Encontradas %u alarmas activas", nalarms);

    return nalarms;


}

