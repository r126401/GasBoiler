

#include "rainmaker_interface.h"
#include <string.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
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
#include "app_network.h"

static const char *TAG = "rainmaker_interface.c";

esp_rmaker_device_t *gasBoiler_device;

static const char *list_mode[] = {TEXT_STATUS_APP_ERROR, TEXT_STATUS_APP_AUTO, TEXT_STATUS_APP_MANUAL, 
    TEXT_STATUS_APP_STARTING, TEXT_STATUS_APP_CONNECTING, TEXT_STATUS_APP_UPGRADING, TEXT_STATUS_APP_UNDEFINED};





/* Callback to handle commands received from the RainMaker cloud */
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{

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
    
    /* Initialize the ESP RainMaker Agent.
     * Note that this should be called after app_network_init() but before app_network_start()
     * */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = true,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, ESP_RMAKER_NODE_NAME, ESP_RMAKER_DEVICE_THERMOSTAT);
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

    /* Create the thermostat device*/
    gasBoiler_device = esp_rmaker_device_create(ESP_RMAKER_NAME_DEVICE, ESP_RMAKER_DEVICE_THERMOSTAT, NULL);
    esp_rmaker_device_add_model(gasBoiler_device, ESP_RMAKER_MODEL_NAME);

    
    /* Definitions of parameters*/

    
    /* name */
    param = esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, ESP_RMAKER_NAME_DEVICE );
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
        "umbral", 
        ESP_RMAKER_PARAM_SETPOINT_TEMPERATURE, 
        esp_rmaker_float(ESP_RMAKER_DEFAULT_SETPOINT_TEMPERATURE),
        PROP_FLAG_READ | PROP_FLAG_WRITE | PROP_FLAG_PERSIST);
        esp_rmaker_device_add_param(gasBoiler_device, param);
        //esp_rmaker_device_assign_primary_param(gasBoiler_device, param);

   
    /* Setpoint AC MODE*/
    param = esp_rmaker_param_create(
        "mode", 
        ESP_RMAKER_PARAM_AC_MODE, 
        esp_rmaker_str(TEXT_STATUS_APP_STARTING),
        PROP_FLAG_READ | PROP_FLAG_WRITE | PROP_FLAG_PERSIST);
        esp_rmaker_param_add_valid_str_list(param, list_mode, 7);
        esp_rmaker_device_add_param(gasBoiler_device, param);

    /* Setpoint TEMP CORRECTION*/
    param = esp_rmaker_param_create(
        "mode", 
        ESP_RMAKER_PARAM_AC_MODE, 
        esp_rmaker_str(TEXT_STATUS_APP_STARTING),
        PROP_FLAG_READ | PROP_FLAG_WRITE | PROP_FLAG_PERSIST);
        esp_rmaker_param_add_valid_str_list(param, list_mode, 7);
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
    esp_rmaker_device_add_cb(gasBoiler_device, write_cb, NULL);  
   
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