


#include "rainmaker_events.h"
//#include "lv_main.h"
#include "events_lcd.h"
#include "esp_log.h"
#include "time.h"
#include "esp_timer.h"
#include "esp_sntp.h"
#include "esp_rmaker_utils.h"

static char *TAG = "rainmaker_events.c";
static esp_timer_handle_t timer_date_text;
const esp_timer_create_args_t text_date_shot_timer_args = {
    .callback = &time_refresh,
    /* name is optional, but may help identify the timer when debugging */
    .name = "time refresh date text"
};


/**
 * @brief Construct a new register device in cloud object
 * 
 * @param register_data 
 */

void  provisioning_device(char *register_data) {


    
    //lv_paint_qr_code(register_data);
    set_lcd_qr_register(register_data);
    set_lcd_update_text_mode(CONFIG_TEXT_STATUS_APP_FACTORY);
    set_lcd_update_bluetooth(true);
    //lv_update_show_bluetooth(true);
    //lv_update_text_mode(CONFIG_TEXT_STATUS_APP_FACTORY);


}

void notify_wifi_status(bool status) {

    ESP_LOGI(TAG, "Status wifi: %d", status);
    set_lcd_update_wifi_status(status);
    if (status == false) {
        set_lcd_update_broker_status(false);
    }


}

void notify_mqtt_status(bool status) {

    ESP_LOGI(TAG, "Status mqtt: %d", status);
    set_lcd_update_broker_status(true);

}




void notify_device_started() {

    ESP_LOGI(TAG, "device initied");
    //lv_update_text_mode(CONFIG_TEXT_STATUS_APP_STARTING);
    set_lcd_update_text_mode(CONFIG_TEXT_STATUS_APP_STARTING);



}


static bool get_now(uint32_t *hour, uint32_t *min, uint32_t *sec) {

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
       /* if (get_app_status() == STATUS_APP_AUTO) {
            lv_update_lcd_schedule(true);
        }
            */
        
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

