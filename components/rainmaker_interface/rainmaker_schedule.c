


#include <string.h>
#include <inttypes.h>
#include "time.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_rmaker_schedule.h"
#include "cJSON.h"
#include "esp_rmaker_internal.h"
#include "app_interface.h"
#include "rainmaker_schedule.h"


static char *TAG = "rainmaker_schedule.c";

uint8_t get_schedules_list() {

    cJSON *json;
    cJSON *schedules;
    cJSON *schedule;
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
    cJSON *device;
    cJSON *obj;

    if ((action = cJSON_GetObjectItem(item, "action")) == NULL) {
        ESP_LOGE(TAG, "Error al extraer el parametro action");
        return ESP_FAIL;
    }

    if ((device = cJSON_GetObjectItem(action, CONFIG_ESP_RMAKER_NAME_DEVICE)) == NULL) {
        ESP_LOGE(TAG, "Error al extraer el nombre del dispositivo: %s", cJSON_Print(action));
        return ESP_FAIL;
    }

    if ((obj = cJSON_GetObjectItem(device, CONFIG_ESP_RMAKER_TYPE_PARAM_SETPOINT_TEMPERATURE_NAME)) == NULL) {
        ESP_LOGE(TAG, "Error al extraer el nombre del setpoint temperature: %s", cJSON_Print(action));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Temperatura extraida del trigger: %.1f", cJSON_GetNumberValue(obj));
    return cJSON_GetNumberValue(obj);


}

static esp_err_t extract_trigger_data(cJSON *trigger, int *min_of_trigger, int *mask_of_trigger, int wday) {

    cJSON *index = NULL;
    cJSON *obj = NULL;


    // Obtenemos los datos del trigger
    int j, tam;
    ESP_LOGE(TAG, "%s. dia de la semana %d", cJSON_Print(trigger), wday);
    tam = cJSON_GetArraySize(trigger);
    if (tam > 1) {
        ESP_LOGW(TAG, "El array trigger tiene mas de un elemento y no estamos preparados para analizarlo");
    }
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

        if (apply_mask_today(wday, *mask_of_trigger) == false) {
            ESP_LOGE(TAG, "La mascara no mapea y este schedule no es para hoy");
            return ESP_FAIL;
        }


        
    }

    return ESP_OK;



}


static void print_schedule(bool enabled, int min_of_trigger, float setpoint_temperature) {

    int hour;
    int minute;

    hour = min_of_trigger/60;
    minute = min_of_trigger%60;
    ESP_LOGI(TAG, "schedule: enabled: %d --- %02d:%02d --- %.1f", enabled, hour, minute, setpoint_temperature);



}

static void print_schedules(int n_schedules, schedules_t *elements) {

    int i;
    for (i=0;i<n_schedules;i++) {

        printf("schedule: %d, %d, %d, %d, %02d:%02d --> %.1f\n", elements[i].index,elements[i].enabled, elements[i].mask_trigger, elements[i].trigger, elements[i].trigger/60, elements[i].trigger%60, elements[i].temperature);
    }
}

/******ordenar schedules */

// Comparador ascendente por trigger
int comparar_por_trigger(const void *a, const void *b) {
    const schedules_t *sa = (const schedules_t *)a;
    const schedules_t *sb = (const schedules_t *)b;

    if (sa->trigger < sb->trigger) return -1;
    if (sa->trigger > sb->trigger) return 1;
    return 0;
}


int get_data_schedules(int *min_of_day, int *min_of_trigger, float *setpoint_temperature) {

    char *param_list;
    cJSON *json;
    cJSON *programacion;
    cJSON *schedules;
    cJSON *item_schedule;
    cJSON *trigger;
    bool enabled;
    int n_schedules;
    int i;
    int mask_of_trigger = 0;
    schedules_t *elements = NULL;
    time_t now;
	struct tm fecha;
    uint8_t n_programs = 0;


    
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
    if (n_schedules == 0) {
        ESP_LOGW(TAG, "No hay ningun schedule. pasaremos a modo manual");
        return n_schedules;
    }

    if (n_schedules > 0) {
        elements = (schedules_t*) calloc(n_schedules, sizeof(schedules_t));
        if (elements == NULL) {
            ESP_LOGE(TAG, "Fallo al reservar memoria para los schedules");
            return ESP_FAIL;
        }
    }

    time(&now);
    localtime_r(&now, &fecha);
    *min_of_day = fecha.tm_hour * 60 + fecha.tm_min;

   //Bucle para recorrer todos los schedules
    for (i=0;i<n_schedules;i++) {

        ESP_LOGE(TAG, "Bucle %d", i);
        item_schedule = cJSON_GetArrayItem(schedules, i);

        //Comprobamos si el schedule esta habilitado
        if ((enabled = cJSON_IsTrue(cJSON_GetObjectItem(item_schedule, "enabled"))) == false) {
            ESP_LOGE(TAG, "Este schedule esta inhibido %s", cJSON_Print(item_schedule));
            continue;
        }

        if ((trigger = cJSON_GetObjectItem(item_schedule, "triggers")) == NULL) {

            ESP_LOGE(TAG, "Error al extraer el trigger");
            return ESP_FAIL;
        }

        if (extract_trigger_data(trigger, min_of_trigger, &mask_of_trigger, fecha.tm_wday) == ESP_FAIL) {
            ESP_LOGW(TAG, "El schedule no esta habilitado y no cuenta");
            continue;
        }


        elements[i].index = i;
        elements[i].enabled = enabled;
        elements[i].mask_trigger = mask_of_trigger;
        elements[i].trigger = *min_of_trigger;
        elements[i].temperature = extract_setpoint_temperature(item_schedule);
        n_programs++;
    }
    
    if (n_programs == 0) {
        ESP_LOGW(TAG, "No hay schedules activos o que mapeen con la mascara.");
        return n_programs;
    }

    if (n_programs == 1) {
        if (*min_of_day >= elements->trigger ) {
            // El trigger ya ha pasado. Cogemos el setpoint temperature y el next schedule pero sería para mañana.
            *min_of_trigger = elements->trigger;
            *setpoint_temperature = elements->temperature;

        } else {
            *min_of_trigger = elements->trigger;
            *setpoint_temperature = CONFIG_DEFAULT_SETPOINT_TEMPERATURE;

        }
    }

    if (n_programs > 1) {
        print_schedules(n_programs, elements);
        // Ordenamos en el mismo array
        qsort(elements, n_programs, sizeof(schedules_t), comparar_por_trigger);
        print_schedules(n_programs, elements);

        //¿Estamos al principio?
        for (i=0;i<n_programs;i++) {

            if (elements[i].trigger > *min_of_day) {
                //item encontrado
                if (i == 0) {
                    //Todos los elementos son posteriores y soy el primero de la lista.
                    //Asigno el proximo intervalo y el anterior setpoint. Es decir, el ultimo del dia anterior.
                    //primer schedule
                    *min_of_trigger = elements[i].trigger;
                    //setpoint del dia anterior
                    *setpoint_temperature = elements[n_programs].temperature;

                } else {
                    // Estoy enmedio ,asi que cojo el schedule y el anterior setpoint
                    *min_of_trigger = elements[i].trigger;
                    *setpoint_temperature = elements[i-1].temperature;
                }
                break;
            }          

            
        }
        // Si llegamos aqui es que es el ultimo de la lista y todos son a pasado. Cogemos el setpoint actual y el schedule de mañana, es decir, el primero de la lista.
        *min_of_trigger = elements->trigger;
        *setpoint_temperature = elements[i-1].temperature;

    }

    ESP_LOGW(TAG, " Hay %d programas activos. Los minutos del dia son %d, y el elemento es elemento es %d y el setpoint %.1f", n_programs, *min_of_day, *min_of_trigger, *setpoint_temperature);


    return n_programs;

}