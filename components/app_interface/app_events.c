

#include "app_events.h"
#include "app_interface.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_log.h"
#include "esp_err.h"
#include "events_lcd.h"
#include "esp_timer.h"


#include <string.h>
#include "strings.h"
#include <stdio.h>



xQueueHandle event_queue_app;
static const char *TAG = "app_events.c";





static void send_alarm(bool status) {

    set_lcd_update_icon_errors(status);
}


static void send_event_app_alarm_off() {

    send_alarm(false);

}

static void send_event_app_alarm_on() {

    send_alarm(true);

}



char* event_app_2mnemonic(EVENT_APP type) {

    static char mnemonic[30];
    switch (type) 
    {

        case EVENT_APP_SETPOINT_THRESHOLD:
            strncpy(mnemonic, "EVENT_APP_SETPOINT_THRESHOLD", 30);
        break;

        case EVENT_APP_TIME_VALID:
            strncpy(mnemonic, "EVENT_APP_TIME_VALID", 30);
        break;
        case EVENT_APP_AUTO:
            strncpy(mnemonic, "EVENT_APP_AUTO", 30);
        break;

        case EVENT_APP_MANUAL:
            strncpy(mnemonic, "EVENT_APP_MANUAL", 30);
        break;
        case EVENT_APP_CONNECTING:
            strncpy(mnemonic, "EVENT_APP_AUTO", 30);
        break;

        case EVENT_APP_CONNECTED:
            strncpy(mnemonic, "EVENT_APP_MANUAL", 30);
        break;
        case EVENT_APP_SYNCING:
            strncpy(mnemonic, "EVENT_APP_AUTO", 30);
        break;

        case EVENT_APP_SYNCED:
            strncpy(mnemonic, "EVENT_APP_MANUAL", 30);
        break;

        case EVENT_APP_ALARM_OFF:
            strncpy(mnemonic, "EVENT_APP_ALARM_OFF", 30);
        break;
        case EVENT_APP_ALARM_ON:
            strncpy(mnemonic, "EVENT_APP_ALARM_ON", 30);
        break;
        case EVENT_APP_FACTORY:
            strncpy(mnemonic, "EVENT_APP_FACTORY", 30);
        break;



    }


        return mnemonic;
}




void receive_event_app(event_app_t event) {

    status_app_t status_app = STATUS_APP_ERROR;


    switch (event.event_app) 
    {

        case EVENT_APP_SETPOINT_THRESHOLD:

            ESP_LOGI(TAG, "Recibido evento EVENT_APP_SETPOINT_THRESHOLD. Threshold = %.1f", event.value); 
            notify_setpoint_temperature(event.value);
            //set_app_update_threshold(event.value, true);
            break;


        case EVENT_APP_TIME_VALID:

            
            break;

        case EVENT_APP_MANUAL:
            status_app = STATUS_APP_MANUAL;
        case EVENT_APP_AUTO:
            status_app = STATUS_APP_AUTO;

        case EVENT_APP_CONNECTING:
            status_app = STATUS_APP_CONNECTING;

        case EVENT_APP_CONNECTED:
            status_app = STATUS_APP_CONNECTED;

        case EVENT_APP_SYNCING:
            status_app = STATUS_APP_SYNCING;

        case EVENT_APP_SYNCED:
            status_app = STATUS_APP_SYNCHRONIZED;
            ESP_LOGW(TAG, "Evento a ser enviado :%s", status2mnemonic(status_app));
            set_status_app(status_app);
            break;

        case EVENT_APP_ALARM_OFF:

        break;
        case EVENT_APP_ALARM_ON:

        break;

        case EVENT_APP_FACTORY:
            set_status_app(STATUS_APP_FACTORY);

        break;




    }

    //ESP_LOGW(TAG, "Retornamos despues de procesar la peticion");

}


void event_app_task(void *arg) {

	event_app_t event;


	event_queue_app = xQueueCreate(10, sizeof(event_app_t));

	for(;;) {
		ESP_LOGI(TAG, "ESPERANDO EVENTO DE APLICACION...Memoria libre: %d", (int) esp_get_free_heap_size());
		if (xQueueReceive(event_queue_app, &event,  portMAX_DELAY) == pdTRUE) {

			receive_event_app(event);


		} else {
			ESP_LOGE(TAG, "NO SE HA PODIDO PROCESAR LA PETICION");
		}

	}
	vTaskDelete(NULL);


}

void create_event_app_task() {



	xTaskCreatePinnedToCore(event_app_task, "event_app_task", /*CONFIG_RESOURCE_EVENT_TASK*/ 1024 * 4, NULL, 0, NULL,0);
	ESP_LOGW(TAG, "TAREA DE EVENTOS DE APLICACION CREADA CREADA");


}


static void send_event_app(event_app_t event) {


	ESP_LOGW(TAG, " envio de evento app %s", event_app_2mnemonic(event.event_app));
	if ( xQueueSend(event_queue_app, &event, pdMS_TO_TICKS(20)) != pdPASS) {
		ESP_LOGE(TAG, "no se ha podido enviar el evento");

	}

}

void send_event_app_alarm(EVENT_APP alarm) {

    event_app_t event;
    event.event_app = alarm;



    switch (alarm) {

        case EVENT_APP_ALARM_ON:
        case EVENT_APP_ALARM_OFF:
            send_event_app(event);
        break;
        default:
            ESP_LOGE(TAG, "Error, no se trata aqui este tipo de alarma");
        break;
    }

}

void send_event_app_setpoint_temperature(float setpoint_temperature) {

    event_app_t event;
    event.event_app = EVENT_APP_SETPOINT_THRESHOLD;
    event.value = setpoint_temperature;
    send_event_app(event);
}

void send_event_app_time_valid() {

    event_app_t event;
    event.event_app = EVENT_APP_TIME_VALID;
    send_event_app(event);

}

void send_event_app_status(EVENT_APP status)  {

    event_app_t event;
    event.event_app = status;

    send_event_app(event);
}

void send_event_app_factory() {

    event_app_t event;
    event.event_app = EVENT_APP_FACTORY;
    send_event_app(event);
}



















