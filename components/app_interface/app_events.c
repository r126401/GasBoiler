

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

        case EVENT_APP_STATUS:
             strncpy(mnemonic, "EVENT_APP_STATUS", 30);
        break;
       
        case EVENT_APP_ALARM_OFF:
            strncpy(mnemonic, "EVENT_APP_ALARM_OFF", 30);
        break;
        case EVENT_APP_ALARM_ON:
            strncpy(mnemonic, "EVENT_APP_ALARM_ON", 30);
        break;

    }


        return mnemonic;
}




void receive_event_app(event_app_t event) {

    status_app_t status_app = STATUS_APP_ERROR;


    switch (event.event_app) 
    {

        case EVENT_APP_SETPOINT_THRESHOLD:

            ESP_LOGI(TAG, "Recibido evento EVENT_APP_SETPOINT_THRESHOLD. Threshold = %.1f", event.value_float); 
            notify_setpoint_temperature(event.value_float);
            
            break;


        case EVENT_APP_TIME_VALID:

            
            break;

        case EVENT_APP_STATUS:
            ESP_LOGW(TAG, "Evento a ser enviado :%s", status2mnemonic(event.value_int));
            set_status_app(event.value_int);
            break;

        case EVENT_APP_ALARM_OFF:

        break;
        case EVENT_APP_ALARM_ON:

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
    event.value_float = setpoint_temperature;
    send_event_app(event);
}

void send_event_app_time_valid() {

    event_app_t event;
    event.event_app = EVENT_APP_TIME_VALID;
    send_event_app(event);

}

void send_event_app_status(status_app_t status)  {

    event_app_t event;
    event.event_app = EVENT_APP_STATUS;
    event.value_int = status;

    send_event_app(event);
}

void send_event_app_factory() {

    event_app_t event;
    event.event_app = EVENT_APP_STATUS;
    send_event_app(event);
}



















