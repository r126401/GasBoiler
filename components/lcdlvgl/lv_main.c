/*
 * lv_main.c
 *
 *  Created on: 26 dic 2025
 *      Author: tuya
 */


#include "lv_main.h"
//#include "lvgl.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "styles.h"
#include "time.h"
#include "esp_log.h"
#include "app_interface.h"


static char *TAG = "lv_main.c";

//objects
lv_obj_t *screen_main_thermostat;
lv_obj_t *layout_notification;
lv_obj_t *date_text;
lv_obj_t *icon_wifi;
lv_obj_t *icon_broker;
lv_obj_t *icon_bluetooth;

lv_obj_t *button_main_reset;
lv_obj_t *label_main_reset;

lv_obj_t *layout_threshold;
lv_obj_t *layout_temperature;
lv_obj_t *icon_threshold;
lv_obj_t *text_threshold;

lv_obj_t *icon_themometer;
lv_obj_t *text_temperature;

lv_obj_t *layout_buttons_threshold;
lv_obj_t *button_mode;
lv_obj_t *button_up;
lv_obj_t *button_down;
lv_obj_t *label_mode;

lv_obj_t *icon_heating;
lv_obj_t *label_text_mode;
lv_obj_t *lv_name_device;

lv_timer_t *timer_time;
lv_timer_t *mytimer;


//styles

lv_style_t style_screen_init_thermostat;
extern lv_display_t * display;




LV_FONT_DECLARE(russo48);

//fonts
static const lv_font_t * normal_font;


LV_IMAGE_DECLARE(ic_threshold);
LV_IMAGE_DECLARE(ic_thermometer);
LV_IMAGE_DECLARE(ic_heating);


float incdec = 0.5;
bool pulse = false;



static void create_text_version(char *version) {

    lv_obj_t *label_version;
    label_version = lv_label_create(screen_main_thermostat);
    //const esp_app_desc_t *app_desc = esp_app_get_description();

    lv_label_set_text_fmt(label_version, "v%s", version);
    lv_obj_set_pos(label_version, lv_pct(90),lv_pct(91.5));


}



/*************************************************************/
/*                 update functions                          */
/*************************************************************/




void lv_update_show_wifi(bool action) {

	//lv_obj_set_flag(icon_wifi, LV_OBJ_FLAG_HIDDEN, action);
	if (action) {
		lv_obj_set_style_opa(icon_wifi, LV_OPA_100, LV_PART_MAIN);
	} else {
		lv_obj_set_style_opa(icon_wifi, LV_OPA_0, LV_PART_MAIN);
	}



}

void lv_update_show_broker(bool action) {


	if (action) {
		lv_obj_set_style_opa(icon_broker, LV_OPA_100, LV_PART_MAIN);
	} else {
		lv_obj_set_style_opa(icon_broker, LV_OPA_0, LV_PART_MAIN);
	}

}


void lv_update_show_bluetooth(bool action) {


	if (action) {
		lv_obj_set_style_opa(icon_bluetooth, LV_OPA_100, LV_PART_MAIN);
	} else {
		lv_obj_set_style_opa(icon_bluetooth, LV_OPA_0, LV_PART_MAIN);
	}

}








/****************************************** Updates functions ***************************************/

void lv_update_time(int hour, int minute) {

    if (date_text != NULL) {

        if ((hour == -1) || (minute == -1)) {
            lv_label_set_text(date_text, "NO TIME!");
            lv_obj_set_style_text_color(date_text, lv_color_hex(LV_COLOR_TEXT_FAIL_NOTIFICATION), LV_PART_MAIN);

         } else {
            lv_label_set_text_fmt(date_text, "%.02d:%.02d", hour, minute);
             lv_obj_set_style_text_color(date_text, lv_color_hex(LV_COLOR_DATE), LV_PART_MAIN);



        }

       

    }

    


}

void lv_update_text_mode(char *mode) {

    if (label_text_mode != NULL) {

        lv_label_set_text_fmt(label_text_mode, "%s", mode);
    }

}

void lv_update_label_mode(char *status) {

    if (label_mode != NULL) {

        lv_label_set_text_fmt(label_mode, "%s", status);

    }

}

void lv_update_temperature(float temperature) {

    char data[10];
    sprintf(data, "%.1f", temperature);
    //ESP_LOGI(TAG, "A pintar en display: %.1f como caracter %s", temperature, data);
    lv_label_set_text_fmt(text_temperature, "%s ºC", data);

}


void lv_update_heating(bool status) {


    if (icon_heating != NULL) {
        if (status == true) {

            lv_obj_remove_flag(icon_heating, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(icon_heating, LV_OBJ_FLAG_HIDDEN);
        }

    }

}

void lv_update_threshold_temperature(float threshold) {

    char data[10];
    sprintf(data, "%.1f", threshold);
    lv_label_set_text_fmt(text_threshold, "%s ºC", data);

}

void lv_update_button_mode(bool enable) {

    if (enable) {
        lv_obj_add_flag(button_mode, LV_OBJ_FLAG_CLICKABLE);
    } else {
        lv_obj_remove_flag(button_mode, LV_OBJ_FLAG_CLICKABLE);

    }
}



/*************************************************************/
/*                 end update functions                      */
/*************************************************************/



static void create_layout_notification() {


	//creating objects
	layout_notification = lv_obj_create(screen_main_thermostat);

	lv_obj_set_pos(layout_notification, 0,0);
	lv_obj_set_size(layout_notification, CONFIG_LCD_H_RES, LV_SIZE_CONTENT);
	configure_style_layout_notification();

	date_text = lv_label_create(screen_main_thermostat);
	lv_update_time(-1, -1);
	lv_label_set_long_mode(date_text, 3);
	lv_obj_set_pos(date_text, CONFIG_LCD_H_RES/2, 0);



	//style objects

    lv_obj_add_style(layout_notification, get_style_layout_notification(), LV_PART_MAIN);
	lv_obj_set_style_pad_all(layout_notification, 5, LV_PART_MAIN);
	lv_obj_set_style_text_font(date_text, &lv_font_montserrat_24, LV_STATE_DEFAULT);



	//position objects
	//lv_obj_set_style_base_dir(layout_notification, LV_BASE_DIR_RTL, LV_PART_MAIN);

	lv_obj_set_flex_flow(layout_notification, LV_FLEX_FLOW_ROW_REVERSE);
	//lv_obj_set_pos(layout_notification,50, 0);
	lv_obj_set_flex_align(layout_notification, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
	lv_obj_align(date_text, LV_ALIGN_TOP_MID, 0, 0);


	//size objects
	//lv_obj_set_size(layout_notification, CONFIG_LCD_H_RES, LV_SIZE_CONTENT);

	//callback functions


	//Icono conexion del dispositivo
    icon_wifi = lv_label_create(layout_notification);
    lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);
    lv_update_show_wifi(false);

	//icono conexion al broker
	icon_broker = lv_label_create(layout_notification);
	lv_label_set_text(icon_broker, LV_SYMBOL_LOOP);
	lv_update_show_broker(false);


	//icono bluetooth
	icon_bluetooth = lv_label_create(layout_notification);
	lv_label_set_text(icon_bluetooth, LV_SYMBOL_BLUETOOTH);
	lv_update_show_bluetooth(false);


}





static void event_handler_button_main_reset(lv_event_t *event) {

    /**
     * Rutina para tratar el reset del dispositivo
     */
	ESP_LOGW(TAG, "reboot...");
    reset_device();

}

static void event_handler_button_factory_reset(lv_event_t *event) {

    /**
     * Rutina para tratar el reset del dispositivo
     */

    //reset_device();
    factory_reset_device();
	ESP_LOGW(TAG, "Factory reset...");

}

static void create_button_reset() {


	button_main_reset = lv_button_create(screen_main_thermostat);
	label_main_reset = lv_label_create(button_main_reset);
	//set_style_buttons_threshold(button_main_reset, button_main_reset);
    lv_obj_add_style(button_main_reset, get_style_buttons_threshold(), 0);
    lv_obj_add_style(button_main_reset, get_style_buttons_threshold_pressed(), LV_STATE_PRESSED);
	lv_label_set_text(label_main_reset, LV_SYMBOL_REFRESH);
    lv_obj_set_pos(button_main_reset, lv_pct(1), lv_pct(2));
    lv_obj_set_size(button_main_reset, SIZE_BUTTONS, SIZE_BUTTONS);
    lv_obj_set_style_text_font(label_main_reset, normal_font, LV_PART_MAIN);
    lv_obj_center(label_main_reset);
    lv_obj_add_event_cb(button_main_reset, event_handler_button_main_reset, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(button_main_reset, event_handler_button_factory_reset, LV_EVENT_LONG_PRESSED, NULL);



}


static void create_layout_threshold() {

	layout_threshold =  lv_obj_create(screen_main_thermostat);
    configure_style_threshold();
    lv_obj_clear_flag(layout_threshold, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(layout_threshold, get_style_layout_threshold(), LV_STATE_DEFAULT);
	lv_obj_align_to(layout_threshold, layout_temperature, LV_ALIGN_OUT_BOTTOM_LEFT, 15, 0);
	//lv_obj_set_flex_align(layout_threshold, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	icon_threshold = lv_image_create(layout_threshold);
	lv_image_set_src(icon_threshold, &ic_threshold);
	text_threshold = lv_label_create(layout_threshold);
	lv_obj_align_to(text_threshold, icon_threshold, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    //lv_update_threshold_temperature(current_threshold);
	//lv_obj_set_pos(text_threshold, 20, 10);
    lv_label_set_text(text_threshold, ".... ºC");
	lv_obj_set_size(layout_threshold, LV_SIZE_CONTENT, LV_SIZE_CONTENT);





}

void create_layout_temperature() {

	//create objects
	layout_temperature = lv_obj_create(screen_main_thermostat);
	icon_themometer = lv_img_create(layout_temperature);
	lv_img_set_src(icon_themometer, &ic_thermometer);
	text_temperature = lv_label_create(layout_temperature);
	lv_label_set_text(text_temperature, "21.5 ºC");

	//style objects
	lv_obj_set_style_text_font(text_temperature, &russo48, LV_PART_MAIN);
    //set_style_layout_temperature();

	//position objects
	lv_obj_center(text_temperature);
	lv_obj_align_to(layout_temperature, layout_notification, LV_ALIGN_OUT_BOTTOM_LEFT, lv_pct(18), CONFIG_LCD_V_RES/4);
	lv_obj_set_flex_align(layout_temperature, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

	//size objects
	lv_obj_set_size(layout_temperature, LV_SIZE_CONTENT, LV_SIZE_CONTENT);


	//callback functions
    //set_style_layout_temperature();
	configure_style_layout_temperature();
    lv_obj_add_style(layout_temperature, get_style_layout_temperature(), LV_STATE_DEFAULT);


}






static void init_main_screen() {

    screen_main_thermostat = lv_screen_active();
   //init_fonts();
    normal_font = LV_FONT_DEFAULT;
    screen_main_thermostat = lv_display_get_screen_active(display);
    lv_obj_set_style_text_font(lv_screen_active(), normal_font, 0);

	lv_style_init(&style_screen_init_thermostat);
	lv_theme_default_init(lv_obj_get_disp(lv_screen_active()),  /*Use the DPI, size, etc from this display*/
					lv_color_hex(0x0534F0), lv_color_hex(0x0534F0),   /*Primary and secondary palette*/
			                                        true,    /*Light or dark mode*/
			                                        LV_FONT_DEFAULT); /*Small, normal, large fonts*/

	lv_obj_add_style(screen_main_thermostat, &style_screen_init_thermostat, LV_PART_MAIN);


}



void timer_cb(lv_timer_t *timer) {


    float *threshold = lv_timer_get_user_data(timer);
    //ESP_LOGI(TAG, "THRESHOLD VALE, %.1f", *threshold);

    //send_event_app_threshold(*threshold);
    notify_setpoint_temperature(*threshold);
    lv_obj_set_style_text_color(text_threshold, lv_color_hex(LV_COLOR_TEXT_NOTIFICATION), LV_PART_MAIN);
    pulse = false;

}


static void lv_event_handler_button_up(lv_event_t *event) {

    /**
     * Rutina de pulsado del boton up
     */

    char *data;
    static float threshold;


    data = (char*) lv_label_get_text(text_threshold);
    sscanf(data, "%f", &threshold);
    threshold += 0.5;


    if (!pulse) {
        pulse = true;
        lv_obj_set_style_text_color(text_threshold, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN);


    } else {
        lv_timer_delete(mytimer);
        mytimer = NULL;
    }

    mytimer = lv_timer_create(timer_cb, 3000, &threshold);
    lv_timer_set_repeat_count(mytimer, 1);
    
    lv_update_threshold_temperature(threshold);






}

static void set_button_threshold_clickable(bool enable) {

    if (enable) {
        lv_obj_add_flag(button_up, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(button_down, LV_OBJ_FLAG_CLICKABLE);

    } else {

        lv_obj_remove_flag(button_up, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(button_down, LV_OBJ_FLAG_CLICKABLE);


    }
}

static void lv_event_handler_button_mode(lv_event_t *event) {

    /**
     * Rutina de pulsado del boton mode
     */
	printf("hika");




    if (strcmp(lv_label_get_text(label_mode), "M") == 0) {


        //lv_update_label_mode("A");
        //lv_update_text_mode("MANUAL");
        set_button_threshold_clickable(false);
        //send_event_app_status(EVENT_APP_MANUAL);



    } else {

        //lv_update_label_mode("M");
        //lv_update_text_mode("AUTO");
        set_button_threshold_clickable(true);
        //send_event_app_status(EVENT_APP_AUTO);

    }




}

static void lv_event_handler_button_down(lv_event_t *event) {

    /**
     * Rutina de pulsado del boton down
     */
    char *data;
    static float threshold;

    data = (char*) lv_label_get_text(text_threshold);
    sscanf(data, "%f", &threshold);
    threshold -= 0.5;


    if (!pulse) {
    pulse = true;
    lv_obj_set_style_text_color(text_threshold, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN);


    } else {
        lv_timer_delete(mytimer);
        mytimer = NULL;
    }

    mytimer = lv_timer_create(timer_cb, 3000, &threshold);
    lv_timer_set_repeat_count(mytimer, 1);

    lv_update_threshold_temperature(threshold);



}



void create_buttons_threshold()
{

	//set_style_buttons_threshold();




    button_up = lv_button_create(layout_buttons_threshold);
    lv_obj_remove_style_all(button_up);                          /*Remove the style coming from the theme*/
    //set_style_buttons_threshold(button_up, button_up);
    lv_obj_add_style(button_up, get_style_buttons_threshold(), 0);
    lv_obj_add_style(button_up, get_style_buttons_threshold_pressed(), LV_STATE_PRESSED);
    lv_obj_set_size(button_up, SIZE_BUTTONS_THRESHOLD, SIZE_BUTTONS_THRESHOLD);
    lv_obj_center(button_up);


    lv_obj_t * label_up = lv_label_create(button_up);
    lv_label_set_text(label_up, LV_SYMBOL_UP);
    lv_obj_center(label_up);

	lv_obj_add_event_cb(button_up, lv_event_handler_button_up, LV_EVENT_CLICKED, &incdec);



	button_mode = lv_button_create(layout_buttons_threshold);
    lv_obj_remove_style_all(button_mode);                          /*Remove the style coming from the theme*/
    //set_style_buttons_threshold(button_mode, button_mode);
    lv_obj_add_style(button_mode, get_style_buttons_threshold(), 0);
    lv_obj_add_style(button_mode, get_style_buttons_threshold_pressed(), LV_STATE_PRESSED);
    lv_obj_set_size(button_mode, SIZE_BUTTONS_THRESHOLD, SIZE_BUTTONS_THRESHOLD);
    lv_obj_center(button_mode);

    label_mode = lv_label_create(button_mode);
    lv_label_set_text(label_mode, "M");
    lv_obj_center(label_mode);
	lv_obj_add_event_cb(button_mode, lv_event_handler_button_mode, LV_EVENT_CLICKED, NULL);
    lv_obj_remove_flag(button_mode, LV_OBJ_FLAG_CLICKABLE);


	button_down = lv_button_create(layout_buttons_threshold);
    lv_obj_remove_style_all(button_down);                          /*Remove the style coming from the theme*/
    //set_style_buttons_threshold(button_down, button_down);
    lv_obj_add_style(button_down, get_style_buttons_threshold(), 0);
    lv_obj_add_style(button_down, get_style_buttons_threshold_pressed(), LV_STATE_PRESSED);
    lv_obj_set_size(button_down, SIZE_BUTTONS_THRESHOLD, SIZE_BUTTONS_THRESHOLD);
    lv_obj_center(button_down);

    lv_obj_t * label_down = lv_label_create(button_down);
    lv_label_set_text(label_down, LV_SYMBOL_DOWN);
    lv_obj_center(label_down);

	lv_obj_add_event_cb(button_down, lv_event_handler_button_down, LV_EVENT_CLICKED, &incdec);



}


static void create_layout_buttons_threshold() {


	layout_buttons_threshold = lv_obj_create(screen_main_thermostat);
	configure_style_layout_button_threshold();
	lv_obj_add_style(layout_buttons_threshold, get_style_layout_buttons_threshold(), LV_PART_MAIN);
	lv_obj_set_flex_flow(layout_buttons_threshold, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_pos(layout_buttons_threshold,lv_pct(84) , lv_pct(15));
	lv_obj_set_size(layout_buttons_threshold, LV_SIZE_CONTENT, LV_SIZE_CONTENT);



	create_buttons_threshold();


}



static void create_heating_icon() {

	icon_heating = lv_image_create(screen_main_thermostat);
	lv_img_set_src(icon_heating, &ic_heating);
	lv_obj_set_pos(icon_heating, lv_pct(70), lv_pct(30));

}


static void create_label_text_mode() {

    lv_obj_t *layout_text_mode = lv_obj_create(screen_main_thermostat);

    lv_obj_set_pos(layout_text_mode, lv_pct(25), lv_pct(25));
    lv_obj_set_size(layout_text_mode, 180, 20);
    label_text_mode = lv_label_create(layout_text_mode);
    lv_obj_center(label_text_mode);
    configure_style_text_mode();
    lv_obj_add_style(layout_text_mode, get_style_text_mode(), LV_PART_MAIN);
    lv_obj_remove_flag(layout_text_mode, LV_OBJ_FLAG_SCROLLABLE);


}


static void lv_create_device_name() {
    
    lv_name_device = lv_label_create(screen_main_thermostat);
    lv_obj_set_pos(lv_name_device,70,5);
    //lv_obj_align_to(lv_name_device, button_main_reset, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_label_set_text(lv_name_device, "Gas Boiler");
}


void create_screen() {

    mytimer = NULL;
	init_main_screen();
	create_text_version("1.0");
	create_layout_notification();
    lv_update_time(-1, -1);
	configure_style_buttons_threshold();
	create_button_reset();
	create_layout_temperature();
    lv_update_temperature(0.0);
	create_layout_threshold();
    lv_update_threshold_temperature(get_setpoint_temperature());
	create_layout_buttons_threshold();
	create_heating_icon();
    lv_update_heating(false);
	create_label_text_mode();
    lv_create_device_name();
    lv_update_device_name(CONFIG_ESP_RMAKER_NAME_DEVICE);
    lv_update_text_mode(CONFIG_TEXT_STATUS_APP_UNKNOWN);
    ESP_LOGI(TAG, "Creada la pantalla principal");
    
}


void lv_paint_qr_code(char *qrcode) {

    // Tamaño del código QR y color
uint16_t qr_size = 80; // Tamaño en píxeles
//lv_color_t qr_fg_color = lv_color_white(); // Color de los píxeles del QR
//lv_color_t qr_bg_color = lv_color_black(); // Color de fondo del QR

// Crear el objeto QR code
lv_obj_t *qr = lv_qrcode_create(screen_main_thermostat);



lv_qrcode_set_size(qr, qr_size);
lv_obj_set_pos(qr, 5,160);


// Posición en pantalla (opcional)
//lv_obj_align(qr, LV_ALIGN_CENTER, 0, 0);

ESP_LOGI(TAG, "qrcode:%s", qrcode);

// Actualizar el QR code con el contenido
lv_qrcode_update(qr, qrcode, strlen(qrcode));
lv_obj_invalidate(qr);


}

void lv_update_device_name(char *device_name) {

     lv_label_set_text(lv_name_device, device_name);

}