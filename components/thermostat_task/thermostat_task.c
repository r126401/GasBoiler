
#include "thermostat_task.h"
#include "app_interface.h"
#include "app_alarms.h"
#include "events_lcd.h"
#include "esp_err.h"
#include "stdbool.h"


#include "onewire_bus.h"
#include "onewire_types.h"
#include "ds18b20.h"
#include "driver/gpio.h"
#include <math.h>
#include "esp_rom_sys.h"
#include "esp_log.h"

//#include "events_app.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

//#include "lv_main.h"


extern EventGroupHandle_t evt_between_task;



//#define ONEWIRE_MAX_DS18B20 1
//#define CONFIG_SENSOR_THERMOSTAT_GPIO 17
//#define CONFIG_RELAY_GPIO 10
static const char *TAG = "thermostat_task";
//TaskHandle_t thermostat_handle = NULL;
ds18b20_device_handle_t ds18b20s[CONFIG_ONEWIRE_MAX_DS18B20];



static void init_ds18b20()
{

    

    // install 1-wire bus
    onewire_bus_handle_t bus = NULL;
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = CONFIG_SENSOR_THERMOSTAT_GPIO,
		.flags = {
            .en_pull_up = true, // enable the internal pull-up resistor in case the external device didn't have one
        }
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10, // 1byte ROM command + 8byte ROM number + 1byte device command
    };
    ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));

    int ds18b20_device_num = 0;
    //ds18b20_device_handle_t ds18b20s[ONEWIRE_MAX_DS18B20];
    onewire_device_iter_handle_t iter = NULL;
    onewire_device_t next_onewire_device;
    esp_err_t search_result = ESP_OK;

    // create 1-wire device iterator, which is used for device search
    ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
    ESP_LOGI(TAG, "Device iterator created, start searching...");
    do {
        search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
        if (search_result == ESP_OK) { // found a new device, let's check if we can upgrade it to a DS18B20
            ds18b20_config_t ds_cfg = {};
			onewire_device_address_t address;
            // check if the device is a DS18B20, if so, return the ds18b20 handle

            if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg, &ds18b20s[ds18b20_device_num]) == ESP_OK) {
				ds18b20_get_device_address(ds18b20s[ds18b20_device_num], &address);
                ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX", ds18b20_device_num, next_onewire_device.address);
                ds18b20_device_num++;
            } else {
                ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
            }
        }
    } while (search_result != ESP_ERR_NOT_FOUND);
    ESP_ERROR_CHECK(onewire_del_device_iter(iter));
    ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", ds18b20_device_num);

    // Now you have the DS18B20 sensor handle, you can use it to read the temperature

}


THERMOSTAT_MODE get_thermostat_mode() {

    return MODE_AUTO;
}





static enum THERMOSTAT_ACTION calcular_accion_termostato(STATUS_RELAY *accion, float current_temperature) {

	float margin_temperature = 0.5;
	float threshold_temperature; // Es el umbral de temperatura para calcular la accion del termostato.

    threshold_temperature = get_setpoint_temperature();


    
    
    //set_lcd_update_threshold_temperature(threshold_temperature);
	ESP_LOGI(TAG, "THRESHOLD : %.1f", threshold_temperature);
	

    if (gpio_get_level(CONFIG_RELAY_GPIO) == OFF) {
               if (current_temperature <= (threshold_temperature - margin_temperature)) {
            	   ESP_LOGI(TAG, "RELE OFF Y SE ENCIENDE. tempMedida: %.2f, tempUmbral: %.02f", current_temperature, threshold_temperature);
                   *accion = ON;
                   return TOGGLE_THERMOSTAT;
               } else {
            	   ESP_LOGI(TAG, "RELE OFF Y DEBE SEGUIR SIGUE OFF. tempMedida: %.2f, tempUmbral: %.02f", current_temperature, threshold_temperature);
                   *accion = OFF;
                   return NO_TOGGLE_THERMOSTAT;

               }
           } else {
               if (current_temperature >= (threshold_temperature + margin_temperature) ) {
            	   ESP_LOGI(TAG, "RELE ON Y SE APAGA. tempMedida: %.2f, tempUmbral: %.02f", current_temperature, threshold_temperature);
                   *accion = OFF;
                   return TOGGLE_THERMOSTAT;
               } else {
            	   ESP_LOGI(TAG, "RELE ON Y DEBE SEGUIR SIGUE ON. tempMedida: %.2f, tempUmbral: %.02f", current_temperature, threshold_temperature);
                   *accion = ON;
                   return NO_TOGGLE_THERMOSTAT;

               }
           }

		

}



THERMOSTAT_ACTION thermostat_action(float current_temperature) {

	STATUS_RELAY accion_rele;
	enum THERMOSTAT_ACTION accion_termostato;
	
	float temperature = -1;

	ESP_LOGI(TAG, "Estamos en thermostat_action");

   status_app_t status;

    status = get_status_gas_boiler();

    if (get_status_gas_boiler() == STATUS_APP_MANUAL) {

        return NO_ACTION_THERMOSTAT;
    }

    temperature = get_current_temperature();

	ESP_LOGI(TAG, "accionar_termostato: LECTURA ANTERIOR: %.2f, LECTURA POSTERIOR: %.2f HA HABIDO CAMBIO DE TEMPERATURA", 
			temperature, current_temperature);

    if (((accion_termostato = calcular_accion_termostato(&accion_rele, current_temperature)) == TOGGLE_THERMOSTAT)) {
    	ESP_LOGI(TAG, "VAMOS A ACCIONAR EL RELE");
    	//*******hacer relay_operation(datosApp, TEMPORIZADA, accion_rele);
		relay_operation(accion_rele);
    } else {
		accion_termostato = NO_TOGGLE_THERMOSTAT;
    }

    
	return accion_termostato;
}



#define DEFAULT_THRESHOLD 21.0




static float redondear_temperatura(float temperatura) {

	float redondeado;
	float diferencia;
	float resultado = 0;
	float valor_absoluto;

	redondeado = lround(temperatura);
	diferencia = temperatura - redondeado;
	//ESP_LOGE(TAG, "temperatura: %.2f, redondeado: %.2f, diferencia: %.2f", temperatura, redondeado, diferencia);
	if (diferencia == 0) {
		resultado = temperatura;
		//ESP_LOGI(TAG, ""TRAZAR"TEMPERATURA ORIGINAL: %.2f, TEMPERATURA REDONDEADA: %.2f,", INFOTRAZA, temperatura, resultado);
		return resultado;

	}
	valor_absoluto = fabs(redondeado);
	//ESP_LOGE(TAG, "temperatura: %.2f, redondeado: %.2f, diferencia: %.2f, valor absoluto :%.2f", temperatura, redondeado, diferencia, valor_absoluto);
	if (diferencia <= 0.25) {
		ESP_LOGI(TAG, "diferencia <= 0.25");
		resultado = valor_absoluto;
	}
	if ((diferencia > 0.25 ) && (diferencia < 0.5)) {
		ESP_LOGI(TAG, "((diferencia > 0.25 ) && (diferencia < 0.5))");
		resultado = valor_absoluto + 0.5;
	}

	if ((diferencia < -0.25)) {
		ESP_LOGI(TAG, "diferencia < -0.25");
		resultado = valor_absoluto - 0.5;
	}

	ESP_LOGI(TAG, "TEMPERATURA ORIGINAL: %.2f, TEMPERATURA REDONDEADA: %.2f,", temperatura, resultado);
	return resultado;

}







static esp_err_t read_temperature(float *temperature_metered) 

{
	esp_err_t error;


	if (ds18b20_trigger_temperature_conversion(*ds18b20s) != ESP_OK) {
		ESP_LOGE(TAG, "Error al hacer la conversion de temperatura");
		return ESP_FAIL;
	}
	if ((error = ds18b20_get_temperature(*ds18b20s, temperature_metered)) == ESP_OK) {

        ESP_LOGI(TAG, "Alarma de sensor off");
		set_alarm(SENSOR_ALARM, ALARM_APP_OFF);

	} else {

        ESP_LOGE(TAG, "Alarma de sensor on");

		set_alarm(SENSOR_ALARM, ALARM_APP_ON);
	}

	

    return error;
    

}




esp_err_t reading_local_temperature(float *current_temperature) {

    esp_err_t error = ESP_FAIL;
	float temperatura_a_redondear;
	//float current_temperature;
	float calibrate;

    calibrate = get_temperature_correction();
    ESP_LOGI(TAG, "Leyendo desde el sensor. Calibrado: %.2f", calibrate);

	error = read_temperature(current_temperature);
	if (error == ESP_OK) {
		ESP_LOGI(TAG," Lectura local correcta!. ");
		temperatura_a_redondear = *current_temperature + calibrate;
		*current_temperature = redondear_temperatura(temperatura_a_redondear);
		thermostat_action(*current_temperature);
		ESP_LOGI(TAG, "Actualizada y enviada la temperatura actualizada: %.2f", *current_temperature);
	} else {
		ESP_LOGE(TAG, " Error al leer desde el sensor de temperatura");
	}

	


	return error;
}



void task_iotThermostat() 
{

	esp_err_t error = ESP_OK ;
	static uint8_t n_errors = 0;
	float current_temperature;
    uint16_t read_interval;


	/**
	 * init driver ds18b20
	 */

	relay_operation(OFF);
	init_ds18b20();
    ESP_LOGI(TAG, "COMIENZA LA TAREA DE LECTURA DE TEMPERATURA");
    while(1) {
        ESP_LOGW(TAG, "Leemos temperatura en local");
        read_interval = get_read_interval();
        error = reading_local_temperature(&current_temperature);
        if (error == ESP_OK) {

            n_errors = 0;
            notify_current_temperature(current_temperature);
            vTaskDelay(read_interval * 1000 / portTICK_PERIOD_MS);
        
        } else {
            n_errors++;
			if (n_errors > CONFIG_ERROR_READING_TEMPERATURE) {
				ESP_LOGE(TAG, "Llevamos %d errores de lectura consecutivos", n_errors);
                notify_sensor_fail();
				

			} else {
                ESP_LOGE(TAG, "%ud Errores al leer la temperatura del dispositivo. Reintentamos en %d segundos", n_errors, read_interval/2);
                vTaskDelay((read_interval/2) * 1000 / portTICK_PERIOD_MS);

            }	
        }

	}
	
	
}







void create_task_thermostat() {

     xTaskCreatePinnedToCore(task_iotThermostat, "tarea_lectura_temperatura", 1024*3, (void*) NULL, 4, NULL,1);
     ESP_LOGW(TAG, "Tarea thermostat task creada");



}


