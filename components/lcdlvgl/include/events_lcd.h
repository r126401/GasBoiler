#include <stdint.h>
#include <stdbool.h>

#pragma once

typedef enum EVENT_TYPE_LCD {

    UPDATE_TIME,
    UPDATE_TEXT_MODE,
    UPDATE_LABEL_MODE,
    UPDATE_TEMPERATURE,
    UPDATE_WIFI_STATUS,
    UPDATE_BROKER_STATUS,
    UPDATE_BLUETOOTH_STATUS,
    UPDATE_HEATING,
    UPDATE_THRESHOLD_TEMPERATURE,
    UPDATE_SCHEDULE,
    UPDATE_ICON_ERRORS,
    UPDATE_TEXT_SCHEDULE,
    UPDATE_BUTTON_MODE,
    UPDATE_PERCENT,
    QR_REGISTER,
    QR_CONFIRMED,
    UPGRADE_FIRMWARE,
    ERROR_FACTORY,
    UPDATE_BUTTON_INSTALATION



}EVENT_TYPE_LCD;

typedef struct event_lcd_t {

    uint32_t par1;
    uint32_t par2;
    uint32_t par3;
    bool status;
    char* text;
    float value;
    EVENT_TYPE_LCD event_type;

}event_lcd_t;


void send_event_lcd(event_lcd_t event);
char* event_lcd_2_mnemonic(EVENT_TYPE_LCD type_lcd);
void init_queue_lcd();
void wait_event_lcd();
void set_lcd_update_time(int par1, int par2, int par3);
void set_lcd_update_text_mode(char *text_mode);
void set_lcd_update_label_mode(char *mode);
void set_lcd_update_temperature(float temperature);
void set_lcd_update_wifi_status(bool status);
void set_lcd_update_broker_status(bool status);
void set_lcd_update_bluetooth(bool status);
void set_lcd_update_heating(bool status);
void set_lcd_update_threshold_temperature(float threshold);
void set_lcd_update_schedule(bool status, int par1, int par2, int par3);
void set_lcd_update_text_schedule(int par1, int par2, int par3);
void set_lcd_update_percent(float percent);
void set_lcd_update_icon_errors(bool status);
void set_lcd_hide_qr_register();
void set_lcd_update_upgrade_firmware(char* message, int cursor);
void set_lcd_update_error_factory();
void set_lcd_update_button_instalation(bool show);
void set_lcd_qr_register(char *register_data);
void set_lcd_update_button_mode_clickable(bool clickable);


