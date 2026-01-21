


#include <string.h>
#include <inttypes.h>
#include "time.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_rmaker_schedule.h"
#include "cJSON.h"
#include "esp_rmaker_internal.h"
#include "app_interface.h"


static char *TAG = "rainmaker_schedule.c";

uint8_t get_schedules_list() {

    cJSON *json;
    cJSON *schedules;
    cJSON *schedule;
    cJSON *triggers = NULL;
    uint8_t n_triggers = 0;



    char *params_list = esp_rmaker_get_node_params();
    ESP_LOGI(TAG, "CONFIGURACION: %s", params_list);

    if (params_list != NULL) {
        json =cJSON_Parse(params_list);
        if (json != NULL) {
            ESP_LOGI(TAG, "Eliminamos los parametros porque ya no hacen falta");
            free(params_list);
            //Localizamos los schedules

            schedule = cJSON_GetObjectItem(json, "Schedule");
            if (schedule == NULL) {
                ESP_LOGW(TAG, "No se han localizado schedules");
            } else {
                schedules = cJSON_GetObjectItem(schedule, "schedules");
                if (schedules != NULL) {
                    n_triggers = cJSON_GetArraySize(schedules);
                    ESP_LOGI(TAG, "Son %ld schedules, y son : %s", n_triggers , cJSON_Print(schedules));
                    cJSON_Delete(json);
                    ESP_LOGI(TAG, "Borrado el json");
                } else {
                    ESP_LOGE(TAG, "Habia etiqueta schedule pero no schedules");
                }

                
            }

        } else {
            ESP_LOGE(TAG, "Error al intentar leer los schedule");
        }
    }

    return n_triggers;
   
}

bool aplica_hoy(uint8_t trigger_d, struct tm *fecha)
{
    uint8_t bit;

    if (fecha->tm_wday == 0) {
        bit = 6;
    } else {
        bit = fecha->tm_wday - 1;
    }

    return (trigger_d & (1 << bit)) != 0;
}

esp_err_t get_next_schedule(int seconds, float setpoint_temperature) {

    char *param_list;
    cJSON *json;
    cJSON *schedule = NULL;
    cJSON *schedules = NULL;
    cJSON *item;
    cJSON *trigger;
    esp_err_t error =  ESP_OK;
    uint32_t hour;
    uint32_t min;
    uint32_t sec;
    uint32_t seconds_of_trigger;
    uint8_t n_triggers;
    uint8_t i;

    time_t now;
	struct tm fecha;


    param_list = esp_rmaker_get_node_params();

    json = cJSON_Parse(param_list);

    if (json == NULL) {

        ESP_LOGE(TAG, "Error al extraer los schedules");
        return ESP_FAIL;
    }

    if ((schedule = cJSON_GetObjectItem(json, "schedule")) == NULL) {

        ESP_LOGW(TAG, "No hay schedules");
        return ESP_FAIL;

    }

    if ((schedules = cJSON_GetObjectItem(schedule, "schedules")) == NULL) {

        ESP_LOGE(TAG, "Habia etiqueta pero no hay schedules");
        return ESP_FAIL;
    }

    n_triggers = cJSON_GetArraySize(schedules);
    for (i=0;i<n_triggers;i++) {
        item = cJSON_GetArrayItem(schedules, i);

        //Comprobamos si el schedule esta habilitado
        if (cJSON_IsTrue(cJSON_GetObjectItem(item, "enable")) == false) {
            ESP_LOGE(TAG, "Este schedule esta inhibido");
            return ESP_FAIL;
        }
        trigger = cJSON_GetObjectItem(item, "triggers");
        //Chequeamos si la mascara de la semana coincide.
        time(&now);
        localtime_r(&now, &fecha);
        //tenemos que buscar los segundos y la mascara para sacar la info del schedule.


        


    }

    




return -1;


    
    
    




}