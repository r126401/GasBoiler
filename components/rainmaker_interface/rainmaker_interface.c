

#include "rainmaker_interface.h"
#include "app_interface.h"
#include <string.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include "esp_err.h"
#include "esp_wifi.h"

#include <esp_event.h>
#include <nvs_flash.h>




#include "esp_rmaker_schedule.h"
#include "esp_rmaker_scenes.h"
#include "esp_rmaker_common_events.h"
#include "esp_rmaker_standard_types.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_services.h"
#include "esp_rmaker_core.h"
#include "esp_rmaker_console.h"
#include "esp_rmaker_ota.h"
#include "app_network.h"
#include "esp_rmaker_utils.h"
#include "esp_rmaker_mqtt.h"
#include "cJSON.h"
#include "esp_rmaker_internal.h"
#include "lv_main.h"
#include "rainmaker_events.h"
#include "events_lcd.h"

static const char *TAG = "rainmaker_interface.c";
char *text_qrcode;

static const char *list_mode[] = {
    CONFIG_TEXT_STATUS_APP_ERROR, 
    CONFIG_TEXT_STATUS_APP_AUTO, 
    CONFIG_TEXT_STATUS_APP_MANUAL, 
    CONFIG_TEXT_STATUS_APP_STARTING, 
    CONFIG_TEXT_STATUS_APP_CONNECTING, 
    CONFIG_TEXT_STATUS_APP_UPGRADING, 
    CONFIG_TEXT_STATUS_APP_UNKNOWN};

esp_rmaker_device_t *gasBoiler_device;

float current_setpoint_temperature = -100;

void platform_notify_current_temperature(float current_temperature) {


    esp_rmaker_param_t *param;

    //set_lcd_update_temperature(current_temperature);
    
    param = esp_rmaker_device_get_param_by_name(gasBoiler_device, ESP_RMAKER_DEF_TEMPERATURE_NAME);
    if (param != NULL) {
        esp_rmaker_param_update_and_report(param, esp_rmaker_float(current_temperature));
    } else {
        ESP_LOGW(TAG, "Rmaker no activo, No se reporta el valor de la temperatura");
    }

}



float platform_get_setpoint_temperature() {

    esp_rmaker_param_t *param;

    if (current_setpoint_temperature == -100) {
        //value in factory reset
        current_setpoint_temperature = CONFIG_DEFAULT_SETPOINT_TEMPERATURE;
    } else {
        param = esp_rmaker_device_get_param_by_name(gasBoiler_device, CONFIG_ESP_RMAKER_TYPE_PARAM_SETPOINT_TEMPERATURE_NAME);
        if (param != NULL) {
            current_setpoint_temperature = esp_rmaker_param_get_val(param)->val.f;
        }
    }

    //ESP_LOGI(TAG, "Setpoint_temperature: %.1f", current_setpoint_temperature);
    
    return current_setpoint_temperature;
}

void platform_notify_sensor_fail() {

    esp_rmaker_param_t *param;
    param = esp_rmaker_device_get_param_by_name(gasBoiler_device, CONFIG_ESP_RMAKER_PARAM_ALARM_NAME);
    if (param != NULL) {
        ESP_LOGW(TAG, "Alarma de sensor en fallo. Se envia el fallo a la cloud");
        esp_rmaker_param_update_and_report(param, esp_rmaker_str(CONFIG_ESP_RMAKER_PARAM_ALARM_SENSOR_FAIL));
    } else {
        ESP_LOGE(TAG, "No se ha podido enviar el fallo a la cloud");
    }
    //set_lcd_update_icon_errors(true);
   
}


int platform_get_read_interval() {

    esp_rmaker_param_t *param;
    int read_interval;
    param = esp_rmaker_device_get_param_by_name(gasBoiler_device, CONFIG_ESP_RMAKER_PARAM_READ_INTERVAL_NAME);
    if (param != NULL) {
        read_interval = esp_rmaker_param_get_val(param)->val.i;
    } else {
        read_interval = DEFAULT_READ_INTERVAL;
    }

    //ESP_LOGI(TAG, "Intervalo de lectura: %d segundos", read_interval);
    return read_interval;


}



float platform_get_current_temperature() {

    esp_rmaker_param_t *param;
    float temperature;

    param = esp_rmaker_device_get_param_by_name(gasBoiler_device, ESP_RMAKER_DEF_TEMPERATURE_NAME);
    if (param != NULL) {

        temperature = esp_rmaker_param_get_val(param)->val.f;
    } else {
        temperature = INVALID_TEMPERATURE;
    }

    //ESP_LOGI(TAG, "Temperatura enviada por la cloud: %.1f", temperature);

    return temperature;
		
}


status_app_t platform_get_status_gas_boiler() {

    return STATUS_APP_AUTO;
}

float platform_get_temperature_correction() {

    float temperature_correction = DEFAULT_TEMPERATURE_CORRECTION;
    
    esp_rmaker_param_t *param;
    param = esp_rmaker_device_get_param_by_name(gasBoiler_device, CONFIG_ESP_RMAKER_PARAM_TEMPERATURE_CORRECTION_NAME);
    if (param != NULL) {
        temperature_correction = esp_rmaker_param_get_val(param)->val.f;

    } else {
        ESP_LOGW(TAG, "Calibracion fallida desde la cloud");
    }

    ESP_LOGI(TAG, "Calibracion de la temperatura en %.1f grados", temperature_correction);

    return temperature_correction;

}

void platform_reset_device() {

    esp_rmaker_reboot(2);

}


void platform_factory_reset_device() {

    
    esp_rmaker_wifi_reset(0,0);
    esp_rmaker_factory_reset(0,0);
}

esp_err_t platform_notify_setpoint_temperature(float setpoint_temperature) {

    esp_rmaker_param_t *param;

    param = esp_rmaker_device_get_param_by_name(gasBoiler_device, CONFIG_ESP_RMAKER_TYPE_PARAM_SETPOINT_TEMPERATURE_NAME);
    if (param != NULL) {
        esp_rmaker_param_update_and_report(param, esp_rmaker_float(setpoint_temperature));
        return ESP_OK;
    } else {

        return ESP_FAIL;
    }
    

}


esp_err_t platform_notify_heating_gas_Boiler(bool action) {

    esp_rmaker_param_t *param;
    param = esp_rmaker_device_get_param_by_name(gasBoiler_device, CONFIG_ESP_RMAKER_PARAM_HEATING_NAME);
    if (param != NULL) {
        esp_rmaker_param_update_and_report(param, esp_rmaker_bool(action));
        //esp_rmaker_param_update_and_report(param, "estadisticas", esp_rmaker_bool(action);
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "El valor del parametro no se ha podido leer");
        return ESP_FAIL;
    }



}

char* platform_get_device_name() {

    char *name;
    esp_rmaker_param_t *param;
    param = esp_rmaker_device_get_param_by_name(gasBoiler_device, ESP_RMAKER_DEF_NAME_PARAM);
    if (param != NULL) {
        name = esp_rmaker_param_get_val(param)->val.s;
        return name;
    } else {
        ESP_LOGE(TAG, "No se ha podido extraer el nombre del dispositivo");
        return NULL;
    }

}


void event_handler_sync (struct timeval *tv) {

    ESP_LOGE(TAG, "Evento de sincronizacion");
    sntp_sync_status_t sync_status = sntp_get_sync_status();
    
    ESP_LOGI(TAG, "SYNSTATUS ES %d", sync_status);

   switch (sync_status) {

    case SNTP_SYNC_STATUS_RESET:
        ESP_LOGW(TAG, "La sincronizacion esta en estado reset");
        break;

    case SNTP_SYNC_STATUS_COMPLETED:
        ESP_LOGI(TAG, "La sincronizacion esta completada");
        update_time_valid(true);
        break;


    case SNTP_SYNC_STATUS_IN_PROGRESS:  
        ESP_LOGE(TAG, "La sincronizacion esta en progreso");
        //set_alarm(NTP_ALARM, ALARM_APP_ON);

    break; // Smooth time sync in progress.
    
   }

}


static void topic_cb (const char *topic, void *payload, size_t payload_len, void *priv_data) {

    cJSON *json;
    cJSON *schedules;

    
    json = cJSON_Parse((char*) payload);

    if (json == NULL) {
        ESP_LOGW(TAG, "El payload recibido no es json");
        return;
    }

    schedules = cJSON_GetObjectItem(json, "Schedule");
    if (schedules != NULL) {

        ESP_LOGW(TAG, "Se ha encontrado una operacion de schedules");
        //esp_timer_create(&update_lcd_schedules_shot_timer_args, &timer_update_lcd);
        //esp_timer_start_once(timer_update_lcd, 1000000);
    } else {
        ESP_LOGE(TAG, "No Se ha encontrado una operacion de schedules");
    }
    


    /**
     * Es necesario extraer la info para refrescar el schedule de la pantalla.
     * {"Schedule":{"Schedules":[{"id":"GO41","operation":"enable"}]}}
     */

    ESP_LOGE(TAG, "Se ha recibido informacion: %.*s", payload_len, (char*) payload);
}


/* Event handler for catching RainMaker events */
static void event_handler_wifi(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
    case IP_EVENT_ETH_GOT_IP:
    case IP_EVENT_AP_STAIPASSIGNED:
        ESP_LOGE(TAG, "Se ha obtenido direccion ip y estamos conectados a internet. Event id: %d", event_id);
        notify_wifi_status(true);
        
        break;
    
    case IP_EVENT_STA_LOST_IP:
    case IP_EVENT_ETH_LOST_IP:
    case WIFI_EVENT_STA_BEACON_TIMEOUT:
        ESP_LOGE(TAG, "Se ha perdido la señal wifi. Event id: %d", event_id);
        notify_wifi_status(false);
        notify_mqtt_status(false);

        break;
    
    default:
    ESP_LOGW(TAG, "El evento %d no se esta tratando en el event_handler_wifi ", event_id);
        break;
    }

    ESP_LOGE(TAG, "Recibido evento de wifi %s, id: %d", event_base, event_id);

}



/* Event handler for catching RainMaker events */
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == RMAKER_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_INIT_DONE:
                ESP_LOGI(TAG, "event_handler_RainMaker Initialised.");
                notify_device_started();
                break;
            case RMAKER_EVENT_CLAIM_STARTED:
                ESP_LOGI(TAG, "event_handler_RainMaker Claim Started.");
                set_status_app(STATUS_APP_CONNECTING);
                break;
            case RMAKER_EVENT_CLAIM_SUCCESSFUL:
                ESP_LOGI(TAG, "event_handler_RainMaker Claim Successful.");
                break;
            case RMAKER_EVENT_CLAIM_FAILED:
                ESP_LOGI(TAG, "event_handler_RainMaker Claim Failed.");
                break;
            case RMAKER_EVENT_LOCAL_CTRL_STARTED:
                ESP_LOGI(TAG, "event_handler_Local Control Started.");
                break;
            case RMAKER_EVENT_LOCAL_CTRL_STOPPED:
                ESP_LOGI(TAG, "event_handler_Local Control Stopped.");
                break;
            default:
                ESP_LOGW(TAG, "event_handler_Unhandled RainMaker Event: %"PRIi32, event_id);
        }
    } else if (event_base == RMAKER_COMMON_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_REBOOT:
                ESP_LOGI(TAG, "event_handler_Rebooting in %d seconds.", *((uint8_t *)event_data));
                break;
            case RMAKER_EVENT_WIFI_RESET:
                ESP_LOGI(TAG, "event_handler_Wi-Fi credentials reset.");
                break;
            case RMAKER_EVENT_FACTORY_RESET:
                ESP_LOGI(TAG, "event_handler_Node reset to factory defaults.");
                break;
            case RMAKER_MQTT_EVENT_CONNECTED:
                ESP_LOGI(TAG, "event_handler_MQTT Connected.");
                notify_mqtt_status(true);
                char *id_node = esp_rmaker_get_node_id();
                char topic[80] = {0};
                sprintf(topic, "node/%s/params/remote", id_node);
                esp_err_t error = esp_rmaker_mqtt_subscribe(topic, topic_cb, 0, NULL);
                break;
            case RMAKER_MQTT_EVENT_DISCONNECTED:
                ESP_LOGI(TAG, "event_handler_MQTT Disconnected.");
                notify_mqtt_status(false);
                break;
            case RMAKER_MQTT_EVENT_PUBLISHED:
                ESP_LOGI(TAG, "event_handler_MQTT Published. Msg id: %d.", *((int *)event_data));
                get_schedules_list();
                break;
            default:
                ESP_LOGW(TAG, "event_handler_Unhandled RainMaker Common Event: %"PRIi32, event_id);
        }
    } else if (event_base == APP_NETWORK_EVENT) {
        switch (event_id) {
            case APP_NETWORK_EVENT_QR_DISPLAY:
                ESP_LOGI(TAG, "event_handler_Provisioning QR : %s", (char *)event_data);
                if (text_qrcode == NULL) {
                    text_qrcode = (char*) calloc(strlen(event_data) + 1, sizeof(char));
                }
                strncpy(text_qrcode, event_data, strlen(event_data));
                print_qr_register(text_qrcode);
                //falta liberar la memoria te text_qrcode cuando se acabe de provisionar
                break;
            case APP_NETWORK_EVENT_PROV_TIMEOUT:
                ESP_LOGI(TAG, "event_handler_Provisioning Timed Out. Please reboot.");
                break;
            case APP_NETWORK_EVENT_PROV_RESTART:
                ESP_LOGI(TAG, "event_handler_Provisioning has restarted due to failures.");
                break;
            default:
                ESP_LOGW(TAG, "event_handler_Unhandled App Wi-Fi Event: %"PRIi32, event_id);
                break;
        }
    } else if (event_base == RMAKER_OTA_EVENT) {
        switch(event_id) {
            case RMAKER_OTA_EVENT_STARTING:
                ESP_LOGI(TAG, "event_handler_Starting OTA.");
                break;
            case RMAKER_OTA_EVENT_IN_PROGRESS:
                ESP_LOGI(TAG, "event_handler_OTA is in progress.");
                break;
            case RMAKER_OTA_EVENT_SUCCESSFUL:
                ESP_LOGI(TAG, "event_handler_OTA successful.");
                break;
            case RMAKER_OTA_EVENT_FAILED:
                ESP_LOGI(TAG, "event_handler_OTA Failed.");
                break;
            case RMAKER_OTA_EVENT_REJECTED:
                ESP_LOGI(TAG, "event_handler_OTA Rejected.");
                break;
            case RMAKER_OTA_EVENT_DELAYED:
                ESP_LOGI(TAG, "event_handler_OTA Delayed.");
                break;
            case RMAKER_OTA_EVENT_REQ_FOR_REBOOT:
                ESP_LOGI(TAG, "event_handler_Firmware image downloaded. Please reboot your device to apply the upgrade.");
                break;
            default:
                ESP_LOGW(TAG, "event_handler_Unhandled OTA Event: %"PRIi32, event_id);
                break;
        }
    } else {
        ESP_LOGW(TAG, "event_handler_Invalid event received!");
    }
}





/* Callback to handle commands received from the RainMaker cloud */
static esp_err_t message_cloud_received(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{

    switch (ctx->src) {

    /** Request triggered in the init sequence i.e. when a value is found
     * in persistent memory for parameters with PROP_FLAG_PERSIST.
     */
    case ESP_RMAKER_REQ_SRC_INIT:
        ESP_LOGI(TAG, "message_cloud_received. ESP_RMAKER_REQ_SRC_INIT");
    

    break;

    /** Request received from cloud */
    case  ESP_RMAKER_REQ_SRC_CLOUD:
        ESP_LOGI(TAG, "message_cloud_received. ESP_RMAKER_REQ_SRC_CLOUD");

    break;

    /** Request received when a schedule has triggered */
    case ESP_RMAKER_REQ_SRC_SCHEDULE:
        ESP_LOGI(TAG, "message_cloud_received. ESP_RMAKER_REQ_SRC_SCHEDULE");
       


    break;
    /** Request received when a scene has been activated */
    case ESP_RMAKER_REQ_SRC_SCENE_ACTIVATE:
        ESP_LOGI(TAG, "message_cloud_received. ESP_RMAKER_REQ_SRC_SCENE_ACTIVATE");


    break;
    /** Request received when a scene has been deactivated */
    case ESP_RMAKER_REQ_SRC_SCENE_DEACTIVATE:
        ESP_LOGI(TAG, "message_cloud_received. ESP_RMAKER_REQ_SRC_SCENE_DEACTIVATE");

    break;
    /** Request received from a local controller */
    case ESP_RMAKER_REQ_SRC_LOCAL:
        ESP_LOGI(TAG, "message_cloud_received. ESP_RMAKER_REQ_SRC_LOCAL");

    break;
    /** Request received via command-response framework */
    case ESP_RMAKER_REQ_SRC_CMD_RESP:
        ESP_LOGI(TAG, "message_cloud_received. ESP_RMAKER_REQ_SRC_CMD_RESP");

    break;
    /** Request initiated from firmware/console commands */
    case ESP_RMAKER_REQ_SRC_FIRMWARE:
        ESP_LOGI(TAG, "message_cloud_received. ESP_RMAKER_REQ_SRC_FIRMWARE");
    
    break;

    /** This will always be the last value. Any value equal to or
     * greater than this should be considered invalid.
     */

     case ESP_RMAKER_REQ_SRC_MAX:
        ESP_LOGI(TAG, "message_cloud_received. ESP_RMAKER_REQ_SRC_MAX");

     break;
        ESP_LOGI(TAG, "message_cloud_received. Default");

     default:

     break;

        
    }

    return ESP_OK;

}





void rainmaker_interface_init_environment() {


    esp_rmaker_param_t *param;

    /* Initialize Application specific hardware drivers and
     * set initial state.
     */
    //app_driver_init();

    /* Initialize NVS. */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    /* Initialize Wi-Fi. Note that, this should be called before esp_rmaker_node_init()
     */
    app_network_init();

    /* Register an event handler to catch RainMaker events */
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_COMMON_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(APP_NETWORK_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(RMAKER_OTA_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler_wifi, NULL));



    /* Initialize the ESP RainMaker Agent.
     * Note that this should be called after app_network_init() but before app_network_start()
     * */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = true,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, CONFIG_ESP_RMAKER_NODE_NAME, ESP_RMAKER_DEVICE_THERMOSTAT);
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

    /* Create the thermostat device*/
    gasBoiler_device = esp_rmaker_device_create(CONFIG_ESP_RMAKER_NAME_DEVICE, ESP_RMAKER_DEVICE_THERMOSTAT, NULL);
    esp_rmaker_device_add_model(gasBoiler_device, CONFIG_ESP_RMAKER_MODEL_NAME);

    
    /* Definitions of parameters*/

    
    /* name */
    param = esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, CONFIG_ESP_RMAKER_NAME_DEVICE );
    //assign_primary_param(param);
    esp_rmaker_device_add_param(gasBoiler_device, param);

    /* power*/

   // param = esp_rmaker_power_param_create(ESP_RMAKER_DEF_POWER_NAME, false);
    //assign_primary_param(param);
    //esp_rmaker_device_add_param(gasBoiler_device, param);
    //esp_rmaker_device_assign_primary_param(gasBoiler_device, param);



    /* temperature*/
    param = esp_rmaker_param_create(
        ESP_RMAKER_DEF_TEMPERATURE_NAME, 
        ESP_RMAKER_PARAM_TEMPERATURE, 
        esp_rmaker_float(22.5),
        PROP_FLAG_READ | PROP_FLAG_WRITE | PROP_FLAG_PERSIST);
        //assign_primary_param(param);
        esp_rmaker_device_add_param(gasBoiler_device, param);
        esp_rmaker_device_assign_primary_param(gasBoiler_device, param);


    /* Setpoint temperature*/
    param = esp_rmaker_param_create(
        CONFIG_ESP_RMAKER_TYPE_PARAM_SETPOINT_TEMPERATURE_NAME, 
        CONFIG_ESP_RMAKER_TYPE_PARAM_SETPOINT_TEMPERATURE, 
        esp_rmaker_float(CONFIG_ESP_RMAKER_DEFAULT_SETPOINT_TEMPERATURE),
        PROP_FLAG_READ | PROP_FLAG_WRITE | PROP_FLAG_PERSIST);
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_param_add_bounds(param, esp_rmaker_float(0.0f), esp_rmaker_float(50.0f), esp_rmaker_float(0.5f));
        esp_rmaker_device_add_param(gasBoiler_device, param);
        
        //esp_rmaker_device_assign_primary_param(gasBoiler_device, param);
    
    /* heating*/
    param = esp_rmaker_param_create(
        CONFIG_ESP_RMAKER_PARAM_HEATING_NAME,
        CONFIG_ESP_RMAKER_PARAM_HEATING,
        esp_rmaker_bool(false),
        PROP_FLAG_READ);
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_TOGGLE);
        //esp_rmaker_device_assign_primary_param(gasBoiler_device, param);
        esp_rmaker_device_add_param(gasBoiler_device, param);


   
    /* Setpoint AC MODE*/
    param = esp_rmaker_param_create(
        CONFIG_ESP_RMAKER_TYPE_PARAM_MODE_NAME, 
        ESP_RMAKER_PARAM_AC_MODE, 
        esp_rmaker_str(CONFIG_TEXT_STATUS_APP_STARTING),
        PROP_FLAG_READ);
        esp_rmaker_param_add_valid_str_list(param, list_mode, 7);
        esp_rmaker_device_add_param(gasBoiler_device, param);

    /* Setpoint TEMP CORRECTION*/
    param = esp_rmaker_param_create(
        CONFIG_ESP_RMAKER_PARAM_TEMPERATURE_CORRECTION_NAME, 
        CONFIG_ESP_RMAKER_PARAM_TEMPERATURE_CORRECTION, 
        esp_rmaker_float(DEFAULT_TEMPERATURE_CORRECTION),
        PROP_FLAG_READ | PROP_FLAG_WRITE | PROP_FLAG_PERSIST);
        esp_rmaker_param_add_ui_type(gasBoiler_device, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_param_add_bounds(param, esp_rmaker_float(0.0), esp_rmaker_float(30.0), esp_rmaker_float(0.5));
        esp_rmaker_device_add_param(gasBoiler_device, param);


        /* Alarm param*/

    param = esp_rmaker_param_create(
        CONFIG_ESP_RMAKER_PARAM_ALARM_NAME, 
        CONFIG_ESP_RMAKER_PARAM_ALARM,
        esp_rmaker_str(DEFAULT_ALARM),
        PROP_FLAG_READ);
        esp_rmaker_device_add_param(gasBoiler_device, param);

       /* Read interval param*/

    param = esp_rmaker_param_create(
        CONFIG_ESP_RMAKER_PARAM_READ_INTERVAL_NAME, 
        CONFIG_ESP_RMAKER_PARAM_READ_INTERVAL,
        esp_rmaker_int(DEFAULT_READ_INTERVAL),
        PROP_FLAG_READ | PROP_FLAG_WRITE | PROP_FLAG_PERSIST);
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_SLIDER);
        esp_rmaker_param_add_bounds(param, esp_rmaker_int(10), esp_rmaker_int(120), esp_rmaker_int(5));
        esp_rmaker_device_add_param(gasBoiler_device, param);



    esp_rmaker_system_serv_config_t system_serv_config = {
     .flags = SYSTEM_SERV_FLAGS_ALL,
     .reboot_seconds = 2,
     .reset_seconds = 2,
     .reset_reboot_seconds = 2
    };

    esp_rmaker_system_service_enable(&system_serv_config);

    /* Add the write callback for the device. We aren't registering any read callback yet as
     * it is for future use.
     */
    esp_rmaker_device_add_cb(gasBoiler_device, message_cloud_received, NULL);  
   
    /* Add this switch device to the node */
    esp_rmaker_node_add_device(node, gasBoiler_device);

    /* Enable OTA */
    esp_rmaker_ota_enable_default();

    /* Enable timezone service which will be require for setting appropriate timezone
     * from the phone apps for scheduling to work correctly.
     * For more information on the various ways of setting timezone, please check
     * https://rainmaker.espressif.com/docs/time-service.html. 
     */
    esp_rmaker_timezone_service_enable();

    /* Enable scheduling. */
    esp_rmaker_schedule_enable();

    /* Enable Scenes */
    esp_rmaker_scenes_enable();

    /* Enable Insights. Requires CONFIG_ESP_INSIGHTS_ENABLED=y */
    //app_insights_enable();


    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();

    err = app_network_set_custom_mfg_data(MGF_DATA_DEVICE_TYPE_SWITCH, MFG_DATA_DEVICE_SUBTYPE_SWITCH);
    /* Start the Wi-Fi.
     * If the node is provisioned, it will start connection attempts,
     * else, it will start Wi-Fi provisioning. The function will return
     * after a connection has been successfully established
     */



    err = app_network_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    } else {

        //set_lcd_update_wifi_status(true);
        //set_alarm(WIFI_ALARM, ALARM_APP_OFF);   
    }



sntp_set_time_sync_notification_cb(event_handler_sync);





}

void get_schedules_list() {

    cJSON *json;
    cJSON *schedules;



    char *params_list = esp_rmaker_get_node_params();
    ESP_LOGI(TAG, "CONFIGURACION: %s", params_list);

    if (params_list != NULL) {
        json =cJSON_Parse(params_list);
        if (json != NULL) {
            ESP_LOGI(TAG, "Eliminamos los parametros porque ya no hacen falta");
            free(params_list);
            //Localizamos los schedules

            schedules = cJSON_GetObjectItem(json, "Schedule");
            if (schedules == NULL) {
                ESP_LOGW(TAG, "No se han localizado schedules");
            } else {
                ESP_LOGI(TAG, "Los schedules son : %s", cJSON_Print(schedules));
                cJSON_Delete(schedules);
                ESP_LOGI(TAG, "Borrado el json");

            }

        } else {
            ESP_LOGE(TAG, "Error al intentar leer los schedules");
        }
    }

    
   
}