/*
 * lv_main.h
 *
 *  Created on: 26 dic 2025
 *      Author: tuya
 */

#ifndef SOURCES_LV_MAIN_H_
#define SOURCES_LV_MAIN_H_

#include <stdbool.h>

//#define CONFIG_LCD_H_RES 480
//#define CONFIG_LCD_V_RES 272


#define SIZE_BUTTONS 50
#define SIZE_BUTTONS_THRESHOLD 55


void create_screen();
void lv_update_show_wifi(bool action);
void lv_update_show_broker(bool action);
void lv_update_show_bluetooth(bool action);
void lv_update_time(int hour, int min);
void lv_update_text_mode(char *mode);
void lv_update_label_mode(char *status);
void lv_update_temperature(float temperature);
void lv_update_heating(bool status);
void lv_update_threshold_temperature(float threshold);
void lv_update_button_mode(bool enable);
void lv_paint_qr_code(char *qrcode);



#endif /* SOURCES_LV_MAIN_H_ */
