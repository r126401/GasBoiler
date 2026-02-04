

#include "app_interface.h"
#include "rainmaker_interface.h"
#include "rainmaker_schedule.h"


#include "thermostat_task.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "esp_log.h"
#include "esp_err.h"
#include "events_lcd.h"
#include "esp_timer.h"
#include "esp_sntp.h"
#include "driver/gpio.h"



#include <string.h>
#include "strings.h"
#include <stdio.h>

static esp_timer_handle_t timer_date_text;
const esp_timer_create_args_t text_date_shot_timer_args = {
    .callback = &time_refresh,
    /* name is optional, but may help identify the timer when debugging */
    .name = "time refresh date text"
};


static const char *TAG = "app_interface.c";
extern float current_threshold;
extern EventGroupHandle_t evt_between_task;
status_app_t current_status;
float current_correction_temperature = -3.5;
int current_read_interval = 60;




void set_wifi_status(int status) {

    ESP_LOGI(TAG, "Status wifi: %d", status);

    switch (status) {

        case 0:
        case 4:
            ESP_LOGE(TAG, "Se ha obtenido direccion ip y estamos conectados a internet. Event id: %d", status);
            set_lcd_update_wifi_status(true);
            if (get_current_status_app(STATUS_APP_CONNECTING)) {
                set_status_app(STATUS_APP_CONNECTED);
            }
        break;

        case 1:
        case 5:
            ESP_LOGE(TAG, "Se ha perdido la señal wifi. Event id: %d", status);
            set_lcd_update_broker_status(false);
            if (get_current_status_app() == STATUS_APP_FACTORY) {
                ESP_LOGW(TAG, "Provisionando el dispositivo");
            set_status_app(STATUS_APP_PROVISIONING);
        }
        break;

        case 43:
            if (get_current_status_app() == STATUS_APP_PROVISIONING) {

                ESP_LOGW(TAG, "Credenciales enviadas al dispositivo");
                set_status_app(STATUS_APP_CONNECTING);
            }
            break;

    }

   


}



void set_mqtt_status(bool status) {

    ESP_LOGI(TAG, "Status mqtt: %d", status);
    set_lcd_update_broker_status(status);
    if (status == true) {
        set_lcd_update_wifi_status(true);
    }

    subscribe_remote_events();


}




void notify_device_started() {

    ESP_LOGI(TAG, "device initied");
    //lv_update_text_mode(CONFIG_TEXT_STATUS_APP_STARTING);
    set_lcd_update_text_mode(CONFIG_TEXT_STATUS_APP_STARTING);



}


bool get_now(uint32_t *hour, uint32_t *min, uint32_t *sec) {

    time_t now;
	struct tm fecha;
    time(&now);
    localtime_r(&now, &fecha);

    *hour = fecha.tm_hour;
    *min = fecha.tm_min;
    *sec = fecha.tm_sec;

    return true;


}


void time_refresh(void *arg) {

    uint32_t hour;
    uint32_t min;
    uint32_t sec;
    uint32_t interval;

  
    
    
    if (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {


        //lv_update_time(-1,-1);
        set_lcd_update_time(-1, -1, 0);
        ESP_LOGI(TAG, "Hora invalida");
        interval = 60;
    } else {
        get_now(&hour, &min, &sec);
        //lv_update_time(hour, min);
        set_lcd_update_time(hour, min, 0);
        interval = 60 - sec;

        
        ESP_LOGI(TAG, "Actualizada la hora: %02d:%02d. proximo intervalo: %d", (int) hour, (int) min, (int) interval);
       if (get_current_status_app() == STATUS_APP_AUTO) {
            set_lcd_update_schedule(true, -1, -1, -1);
        }
            
        
    }

    //get_next_schedule(name, &time_end);

    ESP_ERROR_CHECK(esp_timer_start_once(timer_date_text, (interval * 1000000)));

}





void update_time_valid(bool timevalid) {


    uint32_t hour;
    uint32_t min;
    uint32_t sec;
    uint32_t resto = 0;
    static bool sync = false;


    if (timevalid) {



        if (!sync) {
            get_now(&hour, &min, &sec);
            ESP_ERROR_CHECK(esp_timer_create(&text_date_shot_timer_args, &timer_date_text));
            resto = 60 - sec;
            ESP_ERROR_CHECK(esp_timer_start_once(timer_date_text, (resto * 1000000)));
            ESP_LOGI(TAG, "Actualizada la hora: %02d:%02d. Proximo intervalo :%d segundos", (int) hour, (int) min, (int) resto);

            sync = true;
            set_status_app(STATUS_APP_SYNCHRONIZED);
            //lv_update_time(hour, min);
            set_lcd_update_time(hour, min, 0);
/*
            if (get_app_status() == STATUS_APP_AUTO) {
                lv_update_lcd_schedule(true);

            }
  */          

        } 

    } else {
        //lv_update_time(-1,-1);
        set_lcd_update_time(-1, -1, 0);
        //set_lcd_update_time(-1, -1);
    }




}








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

        case STATUS_APP_CONNECTED:
           strncpy(mnemonic, TEXT_STATUS_APP_CONNECTED, 30);

        break;

        case STATUS_APP_SYNCING:
           strncpy(mnemonic, TEXT_STATUS_APP_SYNCING, 30);

        break;

        case STATUS_APP_SYNCHRONIZED:
            strncpy(mnemonic, TEXT_STATUS_APP_SYNCHRONIZED, 30);

        break;


        case STATUS_APP_UPGRADING:
           strncpy(mnemonic, TEXT_STATUS_APP_UPGRADING, 30);

        break;

        case STATUS_APP_PROVISIONING:
            strncpy(mnemonic, TEXT_STATUS_APP_PROVISIONING, 30);
        break;

        default:
            strncpy(mnemonic, "ERROR STATUS", 30);

        break;

    }

    return mnemonic;


}


status_app_t get_status() {

    return current_status;
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

void notify_current_status_app() {

    platform_notify_current_status_app();


}


void notify_sensor_fail() {
    
    platform_notify_sensor_fail();
    set_lcd_update_icon_errors(true);

}


static bool exists_shcedules(int *min_of_day, int *min_of_trigger, float *setpoint_temperature) {


    //get_data_schedules(min_of_day, min_of_trigger, setpoint_temperature);

    if (get_data_schedules(min_of_day, min_of_trigger, setpoint_temperature) == 0) {
        ESP_LOGI(TAG, "No hay schedules activos");
        return false;
    } else {

        ESP_LOGE(TAG, "Hay schedules activos, Schedule :%d, %d, %.1f",*min_of_day, *min_of_trigger, *setpoint_temperature);
        return true;
    }
}

void notify_update_schedule() {

    int min_of_day;
    int min_of_trigger;
    float setpoint_temperature;

    ESP_LOGI(TAG, "Se ha modificado un schedule. Estado :%s", status2mnemonic(get_current_status_app()));
    if (exists_shcedules(&min_of_day, &min_of_trigger, &setpoint_temperature) == false) {
        ESP_LOGE(TAG, "Error: Deberia haber al menos un schedule, lo han inhibido o han borrado el ultimo");
        if (get_current_status_app() == STATUS_APP_AUTO) {
            ESP_LOGI(TAG, "Vamos a poner el modo manual despues de modificar el schedule");
            set_status_app(STATUS_APP_MANUAL);
        } else {
            ESP_LOGI(TAG, "No estabamos en modo auto y como no hay schedules no hacemos nada. Estado %s", status2mnemonic(get_current_status_app()));
        }
    } else {
        if ((get_current_status_app() == STATUS_APP_AUTO) || (get_current_status_app() == STATUS_APP_MANUAL)) {
            ESP_LOGE(TAG, "Vamos a colocar el termostato en modo auto. Estado %s", status2mnemonic(get_current_status_app()));
            set_status_app(STATUS_APP_AUTO);
        } else {
            ESP_LOGE(TAG, "El estado es erroneo, y es %s", status2mnemonic(get_current_status_app()));
        }

    }
    
    
}

void notify_start_schedule(float setpoint_temperature) {


    int min_of_day;
    int min_of_trigger;
    float setpoint;

    if (get_data_schedules(&min_of_day, &min_of_trigger, &setpoint) <= 0) {
        ESP_LOGE(TAG, "Error: deberia haber schedules");
        return;
    }

    notify_setpoint_temperature(setpoint_temperature);
    set_lcd_update_schedule(true, min_of_day, min_of_trigger, min_of_day);


}


int get_read_interval() {

    int read_interval;
    if ((read_interval = platform_get_read_interval()) > 0) {
        current_read_interval = read_interval;
        
    } else {
        ESP_LOGW(TAG, "Error al leer el read interval desde la cloud y se asigna por defecto");
    }
    ESP_LOGI(TAG, "Intervalo de lectura: %d segundos", read_interval);
    return current_read_interval;
}

float get_temperature_correction() {

    float correction_temperature = current_correction_temperature;
    if ((correction_temperature = platform_get_temperature_correction()) == -100) {
        ESP_LOGW(TAG, "No se pude extraer la calibracion desde la cloud y se asigna el actual");
    } else {
        current_correction_temperature = correction_temperature;
    }
    ESP_LOGI(TAG, "La calibracion de temperatura es de %.1f", current_correction_temperature);
    return current_correction_temperature;
}


status_app_t get_status_gas_boiler() {

    status_app_t status;
    status = platform_get_status_gas_boiler();
    ESP_LOGI(TAG, "El estado de la aplicacion es %s", status2mnemonic(status));

    return status;

}
float get_current_temperature() {

    float temperature;
    temperature = platform_get_current_temperature();
    ESP_LOGI(TAG, "Temperatura enviada por la cloud: %.1f", temperature);

    return temperature;

}


void reset_device() {

    platform_reset_device();
}

void factory_reset_device() {

    platform_factory_reset_device();
}

void notify_setpoint_temperature(float setpoint_temperature) {

    esp_err_t error;
    THERMOSTAT_ACTION action;
 
    
    ESP_LOGE(TAG, "Vamos a enviar el setpoint temperature");
     error = platform_notify_setpoint_temperature(setpoint_temperature);
    if (error == ESP_OK) {

        ESP_LOGI(TAG, "Setpoint temperature de %.1f enviado a la cloud", setpoint_temperature);
    } else {

                ESP_LOGE(TAG, "No se ha podido enviar el setpoint temperature a la cloud");
    }
    set_lcd_update_threshold_temperature(setpoint_temperature);

    thermostat_action(get_current_temperature());
    

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

void notify_status_factory(char* data_register) {

    print_qr_register(data_register);
    set_status_app(STATUS_APP_FACTORY);

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
    set_lcd_update_wifi_status(false);
    set_lcd_update_broker_status(false);
}

static void set_status_connecting() {

    set_lcd_hide_qr_register(true);
    set_lcd_update_text_mode(TEXT_STATUS_APP_CONNECTING);
    
}

static void set_status_syncing() {



    set_lcd_update_text_mode(TEXT_STATUS_APP_SYNCING);


}

static void set_status_manual() {

    set_lcd_update_text_mode(TEXT_STATUS_APP_MANUAL);
    set_lcd_update_button_mode_clickable(false);

}

void set_temperature_correction(float correction_temperature) {

    current_correction_temperature = correction_temperature;

    platform_set_correction_temperature(correction_temperature);


}

void set_read_interval(int read_interval) {

    current_read_interval = read_interval;
    platform_set_read_interval(read_interval);


}

static void set_status_auto(uint32_t min_of_day, uint32_t min_of_trigger, float setpoint_temperature) {


    //Se cambia la etiqueta a AUTO
    //update_time_valid(true);
    set_lcd_update_text_mode(TEXT_STATUS_APP_AUTO);
    //Se actualiza el schedule en el display
    set_lcd_update_schedule(true, min_of_day, min_of_trigger, min_of_day);
    //Se actualiza el setpoint temperatura en el display y se actua sobre la logica del termostato en base al umbral actualizado
    if (setpoint_temperature == -1.0f) setpoint_temperature = get_setpoint_temperature();
    set_lcd_update_threshold_temperature(setpoint_temperature);
    set_lcd_update_button_mode_clickable(true);
    notify_setpoint_temperature(setpoint_temperature);



}








void set_status_app(status_app_t status) {

    int min_of_day;
    int min_of_trigger;
    float setpoint_temperature;


    if (current_status == status) {
        ESP_LOGW(TAG, "El estado actual y el nuevo son iguales: %s", status2mnemonic(status));
        //return;
    }

    ESP_LOGW(TAG, "%s -----------> %s", status2mnemonic(current_status), status2mnemonic(status));
    if ((status == STATUS_APP_UNDEFINED) || (status == STATUS_APP_STARTING)) {
        set_status_starting();
        current_status = status;
        return;
    }

    if (status == STATUS_APP_FACTORY) {
        set_status_factory();
        current_status = status;
        return;

    }

    if (status == STATUS_APP_PROVISIONING) {
        set_lcd_update_text_mode(TEXT_STATUS_APP_PROVISIONING);
        current_status = status;
        return;
    }

    if (status == STATUS_APP_CONNECTING) {
        if (current_status == STATUS_APP_PROVISIONING) {
            //Hemos acabado el registro y vamos a conectarnos.
            set_status_connecting();
            current_status = STATUS_APP_CONNECTING;
            return;
        }
    }

    if (status == STATUS_APP_CONNECTED) {

        if ((current_status == STATUS_APP_AUTO) || (current_status == STATUS_APP_MANUAL)) {
            set_status_app(STATUS_APP_AUTO);
            return;
        } else {
            set_status_syncing();
            current_status = STATUS_APP_SYNCING;
            return;
        }
        

    }

    if (status == STATUS_APP_SYNCHRONIZED) {

        update_time_valid(true);
        if (exists_shcedules(&min_of_day, &min_of_trigger, &setpoint_temperature) == false) {
            set_status_manual();
            current_status = STATUS_APP_MANUAL;
        } else {

            set_status_auto(min_of_day, min_of_trigger, setpoint_temperature);
            current_status = STATUS_APP_AUTO;
        }
        return;
        
    }

    
    

    if (status == STATUS_APP_MANUAL) {
            ESP_LOGI(TAG, "SE PIDE CAMNBIO A MANUAL");

        if (current_status == STATUS_APP_AUTO) {

            //Si estabamos en auto, si lo ponemos en manual, si el rele estaba en ON se pone a OFF y viceversa
            set_lcd_update_schedule(false, -1, -1, -1);
            if (gpio_get_level(CONFIG_RELAY_GPIO) == OFF) {
                relay_operation(ON);
            } else {
                relay_operation(OFF);
            }
            set_lcd_update_text_mode(TEXT_STATUS_APP_MANUAL);
        }
        
        current_status = status;
        return;
    }
  
    

    if (status == STATUS_APP_AUTO) {
        ESP_LOGI(TAG, "SE PIDE CAMNBIO A AUTO");
        if (exists_shcedules(&min_of_day, &min_of_trigger, &setpoint_temperature) == false) {
            set_status_manual();
        } else {

            set_status_auto(min_of_day, min_of_trigger, setpoint_temperature);
            current_status = status;
        }
        return;
        
    }

    //current_status = status;
    //falta notificar el nuevo estado a la cloud

}

status_app_t get_current_status_app() {

    ESP_LOGW(TAG, "Estado actual vale: %s", status2mnemonic(current_status));
    return current_status;
}


STATUS_RELAY relay_operation(STATUS_RELAY op) {

	
	if (gpio_get_level(CONFIG_RELAY_GPIO) == OFF){
		if (op == ON) {
			gpio_set_level(CONFIG_RELAY_GPIO, op);
			ESP_LOGE(TAG, "Accion: OFF->ON");
                notify_heating_gas_Boiler(op);
		} else {
			ESP_LOGE(TAG, "Accion: OFF->OFF");
			}
	} else {

		if (op == ON) {
			ESP_LOGE(TAG, "Accion: ON->ON");
		} else {
			gpio_set_level(CONFIG_RELAY_GPIO, op);
			ESP_LOGE(TAG, "Accion: ON->OFF");
            notify_heating_gas_Boiler(op);

			}
	}



    //set_lcd_update_heating(op);
    
	return gpio_get_level(CONFIG_RELAY_GPIO);
}


