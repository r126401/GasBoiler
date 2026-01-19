

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
status_app_t current_status;




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


status_app_t get_status() {

    return STATUS_APP_ERROR;
}




void notify_current_temperature(float current_temperature) {

    
    platform_notify_current_temperature(current_temperature);
    set_lcd_update_temperature(current_temperature);
    ESP_LOGI(TAG, "Enviada la temperatura al display");

}

float get_setpoint_temperature() {

    float current_setpoint_temperature;
    current_setpoint_temperature = platform_get_setpoint_temperature();
    ESP_LOGI(TAG, "Setpoint_temperature: %.1f", current_setpoint_temperature);
    return current_setpoint_temperature;

}



void notify_sensor_fail() {
    
    platform_notify_sensor_fail();
    set_lcd_update_icon_errors(true);

}


int get_read_interval() {

    int read_interval;
    read_interval = platform_get_read_interval();
    ESP_LOGI(TAG, "Intervalo de lectura: %d segundos", read_interval);
    return read_interval;
}


status_app_t get_status_gas_boiler() {

    status_app_t status;
    status = platform_get_status_gas_boiler();
    ESP_LOGI(TAG, "El estado de la aplicacion es %d", status);

    return status;

}
float get_current_temperature() {

    float temperature;
    temperature = platform_get_current_temperature();
    ESP_LOGI(TAG, "Temperatura enviada por la cloud: %.1f", temperature);

    return temperature;

}
float get_temperature_correction() {
    
    float temperature_correction;

    temperature_correction = platform_get_temperature_correction();
    ESP_LOGI(TAG, "Calibracion de la temperatura en %.1f grados", temperature_correction);
    return temperature_correction;


}

void reset_device() {

    platform_reset_device();
}

void factory_reset_device() {

    platform_factory_reset_device();
}

void notify_setpoint_temperature(float setpoint_temperature) {

    esp_err_t error;
    float current_temperature;
    THERMOSTAT_ACTION action;

    error = platform_notify_setpoint_temperature(setpoint_temperature);
    if (error == ESP_OK) {

        ESP_LOGI(TAG, "Setpoint temperature de %.1f enviado a la cloud", setpoint_temperature);
    } else {

                ESP_LOGE(TAG, "No se ha podido enviar el setpoint temperature a la cloud");
    }

    current_temperature = get_current_temperature();
    thermostat_action(current_temperature);

}

void notify_heating_gas_Boiler(bool action) {

    esp_err_t error;




    error = platform_notify_heating_gas_Boiler(action);
    if (error == ESP_OK) {
        ESP_LOGI(TAG, "Gas Boiler accionado a valor %d", action);
    } else {
        ESP_LOGE(TAG, "No se ha podido reportar el estado a la cloud");
    }
    
    set_lcd_update_heating(action);



}

void print_qr_register(char* register_data) {

    set_lcd_qr_register(register_data);
    set_lcd_update_text_mode(CONFIG_TEXT_STATUS_APP_FACTORY);
    set_lcd_update_bluetooth(true);
}


static void set_status_starting() {

    set_lcd_update_text_mode(TEXT_STATUS_APP_STARTING);
    set_lcd_update_heating(false);
    set_lcd_update_bluetooth(false);
    set_lcd_update_broker_status(false);
    set_lcd_update_wifi_status(false);
}


static void set_status_factory() {

    set_lcd_update_text_mode(TEXT_STATUS_APP_FACTORY);
    set_lcd_update_bluetooth(true);
}

static void set_status_connecting() {

    set_lcd_update_qr_confirmed(false);
    set_lcd_update_text_mode(TEXT_STATUS_APP_CONNECTING);
    
}

void set_status_app(status_app_t status) {

    if ((status == STATUS_APP_UNDEFINED) || (status == STATUS_APP_STARTING)) {
        current_status = status;
        set_status_starting();
        return;
        
    }

    if (status == STATUS_APP_FACTORY) {
        current_status = status;
        return;
    }

    if (status == STATUS_APP_CONNECTING) {
        if (current_status == STATUS_APP_FACTORY) {
            //Hemos acabado el registro y vamos a conectarnos.
            set_lcd_update_qr_confirmed();
        }
    }


}



