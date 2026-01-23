


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

static bool apply_mask_today(int tm_wday, uint8_t rm_mask)
{
    uint8_t bit;

    /* domingo */
    if (tm_wday == 0) {
        bit = 6;
    } else {
        bit = tm_wday - 1;
    }

    return (rm_mask & (1 << bit)) != 0;
}


static float extract_setpoint_temperature(cJSON *item) {

    cJSON *action;
    cJSON *obj;

    if ((action = cJSON_GetObjectItem(item, "action")) == NULL) {
        ESP_LOGE(TAG, "Error al extraer el parametro action");
        return ESP_FAIL;
    }

    if ((obj = cJSON_GetObjectItem(action, CONFIG_ESP_RMAKER_NAME_DEVICE)) == NULL) {
        ESP_LOGE(TAG, "Error al extraer el nombre del dispositivo: %s", cJSON_Print(action));
        return ESP_FAIL;
    }

    if ((obj = cJSON_GetObjectItem(obj, CONFIG_ESP_RMAKER_TYPE_PARAM_SETPOINT_TEMPERATURE_NAME)) == NULL) {
        ESP_LOGE(TAG, "Error al extraer el nombre del dispositivo: %s", cJSON_Print(action));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Temperatura extraida del trigger: %.1f", cJSON_GetNumberValue(obj));
    return cJSON_GetNumberValue(obj);


}

static uint8_t extract_trigger_data(cJSON *trigger, int *min_of_trigger, int *mask_of_trigger) {

    cJSON *index = NULL;
    cJSON *obj = NULL;
    int n_schedules = 0;


    // Obtenemos los datos del trigger
    int j, tam;
    tam = cJSON_GetArraySize(trigger);
    for (j=0;j<tam;j++) {
        index = cJSON_GetArrayItem(trigger, j);
        if ((obj = cJSON_GetObjectItem(index, "m")) == NULL) {
            ESP_LOGE(TAG, "Error al extraer el parametro m: %s", cJSON_Print(index));
            return ESP_FAIL;
        }

        *min_of_trigger = cJSON_GetNumberValue(obj);

        if ((obj = cJSON_GetObjectItem(index, "d")) == NULL) {
            ESP_LOGE(TAG, "Error al extraer el parametro d: %s", cJSON_Print(index));
            return ESP_FAIL;
        }

        *mask_of_trigger = cJSON_GetNumberValue(obj);
    }

    return n_schedules;



}


uint8_t get_next_schedule(int *min_of_day, int *min_of_trigger, float *setpoint_temperature) {

    char *param_list;
    cJSON *json;
    cJSON *programacion = NULL;
    cJSON *schedules = NULL;
    cJSON *item_schedule;
    cJSON *trigger;
    cJSON *obj;
    cJSON *index;
    esp_err_t error =  ESP_OK;
    uint8_t n_schedules;
    uint8_t i;
    int candidate = -1;
    int prev_candidate = -1;
    time_t now;
	struct tm fecha;
    int mask_of_trigger = 0;
    uint8_t triggers = 0;
    float prev_setpoint = -1;


    param_list = esp_rmaker_get_node_params();

    json = cJSON_Parse(param_list);

    if (json == NULL) {

        ESP_LOGE(TAG, "Error al extraer los schedules");
        return ESP_FAIL;
    }

    if ((programacion = cJSON_GetObjectItem(json, "schedule")) == NULL) {

        ESP_LOGW(TAG, "No hay schedules");
        return ESP_FAIL;

    }

    if ((schedules = cJSON_GetObjectItem(programacion, "schedules")) == NULL) {

        ESP_LOGE(TAG, "Habia etiqueta pero no hay schedules");
        return ESP_FAIL;
    }

    
    n_schedules = cJSON_GetArraySize(schedules);
    //Bucle para recorrer todos los schedules
    for (i=0;i<n_schedules;i++) {
        ESP_LOGE(TAG, "Bucle %d", i);
        item_schedule = cJSON_GetArrayItem(schedules, i);

        //Comprobamos si el schedule esta habilitado
        if (cJSON_IsTrue(cJSON_GetObjectItem(item_schedule, "enabled")) == false) {
            ESP_LOGE(TAG, "Este schedule esta inhibido %s", cJSON_Print(item_schedule));
            continue;
        }

        if ((trigger = cJSON_GetObjectItem(item_schedule, "triggers")) == NULL) {

            ESP_LOGE(TAG, "Error al extraer el trigger");
            return ESP_FAIL;
        }

        extract_trigger_data(trigger, min_of_trigger, &mask_of_trigger);
/*
        // Obtenemos los datos del trigger
        int j, tam;
        tam = cJSON_GetArraySize(trigger);
        for (j=0;j<tam;j++) {
            index = cJSON_GetArrayItem(trigger, j);
            if ((obj = cJSON_GetObjectItem(index, "m")) == NULL) {
                ESP_LOGE(TAG, "Error al extraer el parametro m: %s", cJSON_Print(index));
                return ESP_FAIL;
            }

            *min_of_trigger = cJSON_GetNumberValue(obj);

            if ((obj = cJSON_GetObjectItem(index, "d")) == NULL) {
                ESP_LOGE(TAG, "Error al extraer el parametro d: %s", cJSON_Print(index));
                return ESP_FAIL;
            }

            mask_of_trigger = cJSON_GetNumberValue(obj);
        }
*/
        //Chequeamos si la mascara de la semana coincide.
        time(&now);
        localtime_r(&now, &fecha);
        if (apply_mask_today(fecha.tm_wday, mask_of_trigger) == false) {

            ESP_LOGE(TAG, "La mascara no mapea y este schedule no es para hoy");
            continue;
        }

        *min_of_day = fecha.tm_hour * 60 + fecha.tm_min;
        ESP_LOGI(TAG, "min_of_day: %d, min_of_trigger: %d", min_of_day, min_of_trigger);
        triggers++;
        if (*min_of_trigger <= *min_of_day) {
            ESP_LOGW(TAG, "trigger anterior a la hora actual");
            if (prev_candidate == -1) {
                ESP_LOGW(TAG, "Primer schedule previo");
                prev_candidate = *min_of_trigger;
                prev_setpoint = extract_setpoint_temperature(item_schedule);
                ESP_LOGW(TAG, "Primer schedule previo. prev_setpoint: %.1f", prev_setpoint);
            } else {
                if (prev_candidate > *min_of_trigger) {
                    ESP_LOGW(TAG, "Nuevo candidato para el prev_schedule");
                    prev_candidate = *min_of_trigger;
                    prev_setpoint = extract_setpoint_temperature(item_schedule);
                } else {
                    ESP_LOGW(TAG, "Seguimos iterando para el prev");
                }
            }


            
        } else {
            ESP_LOGW(TAG, "trigger posterior a la hora actual");
            if (candidate == -1 ) {
                candidate = *min_of_trigger;
                *setpoint_temperature = extract_setpoint_temperature(item_schedule);
            } else {
                if (candidate < *min_of_trigger) {
                    ESP_LOGW(TAG, "Nuevo candidato con indice %d", i);
                    candidate = *min_of_trigger;
                    *setpoint_temperature = extract_setpoint_temperature(item_schedule);

                } else {
                    ESP_LOGW(TAG, "Seguimos iterando");
                }
            }
        } 


    }
    ESP_LOGI(TAG, "Candidate vale %d", candidate);
    if (candidate > 0) {
        *min_of_trigger = candidate;
        ESP_LOGE(TAG, "posterior: triggers %d, *min_of_trigger : %d, setpoint_temperature : %.1f", triggers, *min_of_trigger, *setpoint_temperature);
    } else {
        ESP_LOGE(TAG, "previo: triggers %d, *min_of_trigger : %d, setpoint_temperature : %.1f", triggers, prev_candidate, prev_setpoint);
        *setpoint_temperature = prev_setpoint;
        *min_of_trigger = prev_candidate;
     

    }

    return triggers;

}