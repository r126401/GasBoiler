

#include "app_interface.h"
#include "rainmaker_interface.h"

#include "thermostat_task.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_log.h"
#include "esp_err.h"
#include "events_lcd.h"

#include <string.h>
#include "strings.h"
#include <stdio.h>



xQueueHandle event_queue_app;
static const char *TAG = "events_app";
extern float current_threshold;
extern EventGroupHandle_t evt_between_task;




void init_app_environment() {
    
    rainmaker_interface_init_environment();

}




static void send_alarm(bool status) {

    set_lcd_update_icon_errors(status);
}


static void send_event_app_alarm_off() {

    send_alarm(false);

}

static void send_event_app_alarm_on() {

    send_alarm(true);

}






char* status2mnemonic(status_app_t status) {

    static char mnemonic[30];

    switch(status) {

        case STATUS_APP_FACTORY:
            strncpy(mnemonic, TEXT_STATUS_APP_FACTORY, 30);
        break;

        case STATUS_APP_ERROR:
            strncpy(mnemonic, TEXT_STATUS_APP_ERROR, 30);
        break;

        case STATUS_APP_AUTO:
           strncpy(mnemonic, TEXT_STATUS_APP_AUTO, 30);
        break;
        case STATUS_APP_MANUAL:
           strncpy(mnemonic, TEXT_STATUS_APP_MANUAL, 30);

        break;

        case STATUS_APP_STARTING:
           strncpy(mnemonic, TEXT_STATUS_APP_STARTING, 30);

        break;

        case STATUS_APP_CONNECTING:
           strncpy(mnemonic, TEXT_STATUS_APP_CONNECTING, 30);

        break;

        case STATUS_APP_SYNCING:
           strncpy(mnemonic, TEXT_STATUS_APP_SYNCING, 30);

        break;

        case STATUS_APP_UPGRADING:
           strncpy(mnemonic, TEXT_STATUS_APP_UPGRADING, 30);

        break;

        default:
            strncpy(mnemonic, "ERROR STATUS", 30);

        break;

    }

    return mnemonic;


}


status_app_t get_app_status() {

    return STATUS_APP_ERROR;
}




