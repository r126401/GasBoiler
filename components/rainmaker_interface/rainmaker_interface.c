

#include "rainmaker_interface.h"
#include <string.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include "esp_err.h"

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

static const char *TAG = "rainmaker_interface.c";

esp_rmaker_device_t *gasBoiler_device;

static const char *list_mode[] = {CONFIG_TEXT_STATUS_APP_ERROR, CONFIG_TEXT_STATUS_APP_AUTO, CONFIG_TEXT_STATUS_APP_MANUAL, 
    CONFIG_TEXT_STATUS_APP_STARTING, CONFIG_TEXT_STATUS_APP_CONNECTING, CONFIG_TEXT_STATUS_APP_UPGRADING, CONFIG_TEXT_STATUS_APP_UNKNOWN};



/* Event handler for catching RainMaker events */
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == RMAKER_EVENT) {
        switch (event_id) {
            case RMAKER_EVENT_INIT_DONE:
                ESP_LOGI(TAG, "event_handler_RainMaker Initialised.");
                break;
            case RMAKER_EVENT_CLAIM_STARTED:
                ESP_LOGI(TAG, "event_handler_RainMaker Claim Started.");
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
                break;
            case RMAKER_MQTT_EVENT_DISCONNECTED:
                ESP_LOGI(TAG, "event_handler_MQTT Disconnected.");
                break;
            case RMAKER_MQTT_EVENT_PUBLISHED:
                ESP_LOGI(TAG, "event_handler_MQTT Published. Msg id: %d.", *((int *)event_data));
                break;
            default:
                ESP_LOGW(TAG, "event_handler_Unhandled RainMaker Common Event: %"PRIi32, event_id);
        }
    } else if (event_base == APP_NETWORK_EVENT) {
        switch (event_id) {
            case APP_NETWORK_EVENT_QR_DISPLAY:
                ESP_LOGI(TAG, "event_handler_Provisioning QR : %s", (char *)event_data);
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

    param = esp_rmaker_power_param_create(ESP_RMAKER_DEF_POWER_NAME, false);
    //assign_primary_param(param);
    esp_rmaker_device_add_param(gasBoiler_device, param);
    //esp_rmaker_device_assign_primary_param(gasBoiler_device, param);



    /* Setpoint temperature*/
    param = esp_rmaker_param_create(
        "temperature", 
        ESP_RMAKER_DEVICE_THERMOSTAT, 
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
        //esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_SLIDER);
        //esp_rmaker_param_add_bounds(param, esp_rmaker_float(0.0f), esp_rmaker_float(50.0f), esp_rmaker_float(0.5f));

        esp_rmaker_device_add_param(gasBoiler_device, param);
        
        //esp_rmaker_device_assign_primary_param(gasBoiler_device, param);
        

   
    /* Setpoint AC MODE*/
    param = esp_rmaker_param_create(
        CONFIG_ESP_RMAKER_TYPE_PARAM_MODE_NAME, 
        ESP_RMAKER_PARAM_AC_MODE, 
        esp_rmaker_str(CONFIG_TEXT_STATUS_APP_STARTING),
        PROP_FLAG_READ | PROP_FLAG_WRITE | PROP_FLAG_PERSIST);
        esp_rmaker_param_add_valid_str_list(param, list_mode, 7);
        esp_rmaker_device_add_param(gasBoiler_device, param);

    /* Setpoint TEMP CORRECTION*/
    param = esp_rmaker_param_create(
        CONFIG_ESP_RMAKER_PARAM_TEMPERATURE_CORRECTION_NAME, 
        CONFIG_ESP_RMAKER_PARAM_TEMPERATURE_CORRECTION, 
        esp_rmaker_float(DEFAULT_TEMPERATURE_CORRECTION),
        PROP_FLAG_READ | PROP_FLAG_WRITE | PROP_FLAG_PERSIST);
        esp_rmaker_device_add_param(gasBoiler_device, param);




    esp_rmaker_system_serv_config_t system_serv_config = {
     .flags = SYSTEM_SERV_FLAGS_ALL,
     .reboot_seconds = 2,
     .reset_seconds = 2,
     .reset_reboot_seconds = 2
};


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






}