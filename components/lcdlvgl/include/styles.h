/*
 * styles.h
 *
 *  Created on: 27 dic 2025
 *      Author: tuya
 */

#ifndef SOURCES_STYLES_H_
#define SOURCES_STYLES_H_


#include "lvgl.h"

#define LV_COLOR_TEXT_NOTIFICATION 0x81D1F9
#define LV_COLOR_TEXT_FAIL_NOTIFICATION 0XFF0000
#define LV_COLOR_DATE 0XFFFFFF

void configure_style_threshold();
void configure_style_buttons_threshold();
void configure_style_layout_button_threshold();
void configure_style_layout_temperature();
void configure_style_text_mode();
void configure_style_layout_notification();
void configure_style_layout_schedule();
void configure_style_schedule();
lv_style_t * get_style_buttons_threshold();
lv_style_t * get_style_buttons_threshold_pressed();
lv_style_t * get_style_layout_threshold();
lv_style_t * get_style_layout_temperature();
lv_style_t * get_style_layout_buttons_threshold();
lv_style_t * get_style_text_mode();
lv_style_t * get_style_layout_notification();
lv_style_t *get_style_layout_schedule();
lv_style_t *get_style_schedule();



#endif /* SOURCES_STYLES_H_ */
