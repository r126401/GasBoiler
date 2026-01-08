#include "lv_main_thermostat.h"
#include "lv_factory_thermostat.h"
#include "lv_styles_thermostat.h"
#include "lvgl.h"
#include <string.h>
#include "strings.h"
#include <stdio.h>
#include <esp_sntp.h>
#include "esp_log.h"
//#include "app_main.h"
#include "events_app.h"
#include "events_lcd.h"
#include "esp_app_desc.h"



//#define CONFIG_LCD_H_RES 480
//#define CONFIG_LCD_V_RES 272
#define SIZE_BUTTONS_THRESHOLD 55

static const lv_font_t * font_normal;

lv_style_t style_layout_notification;
lv_style_t style_layout_temperature;
lv_style_t style_layout_threshold;
lv_style_t style_layout_buttons_threshold;
lv_style_t style_buttons_threshold;
lv_style_t style_buttons_threshold_pressed;
lv_style_t style_text_mode;
lv_style_t style_reset;
lv_style_t style_schedule;
lv_style_t style_layout_schedule;
lv_style_t style_ok;
lv_style_t style_fail;
lv_style_t style_screen_init_thermostat;


lv_obj_t *layout_notification;
lv_obj_t *screen_main_thermostat;
lv_obj_t *date_text;
lv_obj_t *icon_wifi;
lv_obj_t *icon_broker;
lv_obj_t *layout_temperature;
lv_obj_t *icon_themometer;
lv_obj_t *text_temperature;
lv_obj_t *layout_threshold;
lv_obj_t *icon_threshold;
lv_obj_t *text_threshold;
lv_obj_t *icon_heating;
lv_obj_t *layout_buttons_threshold;
lv_obj_t *button_up;
lv_obj_t *button_down;
lv_obj_t *button_mode;
lv_obj_t *button_main_reset;
lv_obj_t *label_main_reset;
lv_obj_t *layout_schedule;
lv_obj_t *text_from_schedule;
lv_obj_t *progress_schedule;
lv_obj_t *text_to_schedule;
lv_obj_t *icon_warning;
lv_obj_t *icon_instalation;




lv_obj_t *label_text_mode;
lv_obj_t * label_mode;
lv_obj_t *label_percent;
bool pulse = false;
lv_timer_t *mytimer;

extern char *text_qrcode;
//extern float current_threshold;
static const char *TAG = "lv_main_thermostat";



LV_IMAGE_DECLARE(ic_thermometer);
LV_IMAGE_DECLARE(ic_threshold);
LV_IMAGE_DECLARE(ic_heating);
LV_IMAGE_DECLARE(ic_warning);



//LV_FONT_CUSTOM_DECLARE(lv_font_led_30);
//LV_FONT_DECLARE(russo48);
//LV_FONT_DECLARE(russo36);
//LV_FONT_DECLARE(russo100);


float incdec = 0.5;


extern lv_display_t * display;




int lv_update_lcd_schedule(bool status) {


    int index=0;
    //uint32_t hour;
    //index = get_next_schedule(&hour);
    //set_lcd_update_schedule(status, hour, index);
    return index;

}


static void create_layout_notification() {


	//creating objects
	layout_notification = lv_obj_create(screen_main_thermostat);

	date_text = lv_label_create(screen_main_thermostat);
	lv_label_set_text_fmt(date_text, "--:--");
	lv_label_set_long_mode(date_text, 3);
	lv_obj_set_pos(date_text, CONFIG_LCD_H_RES/2, 0);

	//style objects
    lv_obj_add_style(layout_notification, &style_layout_notification, LV_PART_MAIN);
	lv_obj_set_style_pad_all(layout_notification, 5, LV_PART_MAIN);
	lv_obj_set_style_text_font(date_text, &lv_font_montserrat_24, LV_STATE_DEFAULT);



	//position objects

	lv_obj_set_pos(layout_notification,0, 0);
	lv_obj_set_flex_flow(layout_notification, LV_FLEX_FLOW_ROW_REVERSE);
	lv_obj_set_flex_align(layout_notification, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
	lv_obj_align(date_text, LV_ALIGN_TOP_MID, 0, 0);


	//size objects
	lv_obj_set_size(layout_notification, CONFIG_LCD_H_RES, LV_SIZE_CONTENT);

	//callback functions


	//Icono conexion del dispositivo
    icon_wifi = lv_label_create(layout_notification);
    lv_label_set_text(icon_wifi, LV_SYMBOL_WIFI);

	//icono conexion al broker
	icon_broker = lv_label_create(layout_notification);
	lv_label_set_text(icon_broker, LV_SYMBOL_SHUFFLE);



}


void create_layout_temperature() {

	//create objects
	layout_temperature = lv_obj_create(screen_main_thermostat);
	icon_themometer = lv_img_create(layout_temperature);
	lv_img_set_src(icon_themometer, &ic_thermometer);
	text_temperature = lv_label_create(layout_temperature);
	lv_label_set_text(text_temperature, "--.- ºC");

	//style objects
	lv_obj_set_style_text_font(text_temperature, &russo48, LV_PART_MAIN);
    set_style_layout_temperature();

	//position objects
	lv_obj_center(text_temperature);
	lv_obj_align_to(layout_temperature, layout_notification, LV_ALIGN_OUT_BOTTOM_LEFT, lv_pct(18), CONFIG_LCD_V_RES/4);
	lv_obj_set_flex_align(layout_temperature, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

	//size objects
	lv_obj_set_size(layout_temperature, LV_SIZE_CONTENT, LV_SIZE_CONTENT);


	//callback functions
    set_style_layout_temperature();
    lv_obj_add_style(layout_temperature, &style_layout_temperature, LV_STATE_DEFAULT);

}




void create_layout_threshold() {

	layout_threshold =  lv_obj_create(screen_main_thermostat);
    set_style_threshold();
    lv_obj_clear_flag(layout_threshold, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(layout_threshold, &style_layout_threshold, LV_STATE_DEFAULT);
	lv_obj_align_to(layout_threshold, layout_temperature, LV_ALIGN_OUT_BOTTOM_LEFT, 15, 0);
	//lv_obj_set_flex_align(layout_threshold, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	icon_threshold = lv_image_create(layout_threshold);
	lv_image_set_src(icon_threshold, &ic_threshold);
	text_threshold = lv_label_create(layout_threshold);
	lv_obj_align_to(text_threshold, icon_threshold, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    //lv_update_threshold_temperature(current_threshold);
	//lv_obj_set_pos(text_threshold, 20, 10);
    //lv_label_set_text(text_threshold, "25.1 ºC");
	lv_obj_set_size(layout_threshold, LV_SIZE_CONTENT, LV_SIZE_CONTENT);





}

static void create_heating_icon() {

	icon_heating = lv_image_create(screen_main_thermostat);
	lv_img_set_src(icon_heating, &ic_heating);



	lv_obj_set_pos(icon_heating, lv_pct(70), lv_pct(30));



}

void timer_cb(lv_timer_t *timer) {

    
    float *threshold = lv_timer_get_user_data(timer);
    ESP_LOGI(TAG, "THRESHOLD VALE, %.1f", *threshold);

    send_event_app_threshold(*threshold);

    
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


    


    if (strcmp(lv_label_get_text(label_mode), "M") == 0) {


        lv_update_label_mode("A");
        lv_update_text_mode("MANUAL");
        set_button_threshold_clickable(false);
        send_event_app_status(EVENT_APP_MANUAL);


        
    } else {

        lv_update_label_mode("M");
        lv_update_text_mode("AUTO");
        set_button_threshold_clickable(true);
        send_event_app_status(EVENT_APP_AUTO);

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





static void event_handler_button_close(lv_event_t *e) {


    lv_obj_t * btn = lv_event_get_current_target(e);
    lv_obj_t * mbox = lv_obj_get_parent(lv_obj_get_parent(btn));
    lv_msgbox_close(mbox);

}

static void create_msgbox_errors() {

    lv_obj_t *box;
    lv_obj_t *table_status;

    int size_x = CONFIG_LCD_H_RES;
    int size_y = CONFIG_LCD_V_RES;


    box = lv_msgbox_create(screen_main_thermostat);
    lv_obj_set_size(box, size_x, size_y);
    lv_msgbox_add_title(box, "Estado del termostato");
    lv_obj_set_pos(box, 0,0);
    style_status_ok();
    lv_obj_add_style(box, &style_ok, LV_PART_MAIN);
    table_status = lv_table_create(box);
    lv_obj_add_style(table_status, &style_ok, LV_PART_ITEMS);
    lv_obj_set_size(table_status, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_table_set_column_width(table_status, 0, size_x -100);
    lv_table_set_column_width(table_status, 1, size_x - 400);
    lv_table_set_row_count(table_status, 4);


    lv_table_set_cell_value(table_status, 0,0, "Conexion Wifi");

    //lv_color_t color = lv_obj_get_style_text_color(icon_wifi, LV_PART_MAIN);

    if (lv_color_eq(lv_obj_get_style_text_color(icon_wifi, LV_PART_MAIN), lv_color_hex(LV_COLOR_TEXT_NOTIFICATION))) {
        lv_table_set_cell_value(table_status, 0, 1, LV_SYMBOL_OK);
    } else {
        lv_table_set_cell_value(table_status, 0, 1, LV_SYMBOL_CLOSE);
    }

    lv_table_set_cell_value(table_status, 1,0, "Conexion al broker");

    if (lv_color_eq(lv_obj_get_style_text_color(icon_broker, LV_PART_MAIN), lv_color_hex(LV_COLOR_TEXT_NOTIFICATION))) {
        lv_table_set_cell_value(table_status, 1, 1, LV_SYMBOL_OK);
    } else {
        lv_table_set_cell_value(table_status, 1, 1, LV_SYMBOL_CLOSE);
    }


    
    lv_table_set_cell_value(table_status, 2,0, "Sincronizacion de reloj");

     if (lv_color_eq(lv_obj_get_style_text_color(date_text, LV_PART_MAIN), lv_color_hex(LV_COLOR_TEXT_NOTIFICATION))) {
        lv_table_set_cell_value(table_status, 2, 1, LV_SYMBOL_OK);
    } else {
        lv_table_set_cell_value(table_status, 2, 1, LV_SYMBOL_CLOSE);
    }

    

    lv_table_set_cell_value(table_status, 3,0, "Sensor de temperatura");
    lv_table_set_cell_value(table_status, 3, 1, LV_SYMBOL_OK);
   

    lv_obj_t *close = lv_msgbox_add_footer_button(box, "Salir");
    lv_obj_add_event_cb(close, event_handler_button_close, LV_EVENT_CLICKED, NULL);

    



    





}

static void lv_event_handler_button_warning(lv_event_t *event) {

    /**
     * Rutina para tratar las alarmas del dispositivo
     */

 
    create_msgbox_errors();



}



void create_buttons_threshold()
{

	set_style_buttons_threshold();




    button_up = lv_button_create(layout_buttons_threshold);
    lv_obj_remove_style_all(button_up);                          /*Remove the style coming from the theme*/
    lv_obj_add_style(button_up, &style_buttons_threshold, 0);
    lv_obj_add_style(button_up, &style_buttons_threshold_pressed, LV_STATE_PRESSED);
    lv_obj_set_size(button_up, SIZE_BUTTONS_THRESHOLD, SIZE_BUTTONS_THRESHOLD);
    lv_obj_center(button_up);


    lv_obj_t * label_up = lv_label_create(button_up);
    lv_label_set_text(label_up, LV_SYMBOL_UP);
    lv_obj_center(label_up);

	lv_obj_add_event_cb(button_up, lv_event_handler_button_up, LV_EVENT_CLICKED, &incdec);



	button_mode = lv_button_create(layout_buttons_threshold);
    lv_obj_remove_style_all(button_mode);                          /*Remove the style coming from the theme*/
    lv_obj_add_style(button_mode, &style_buttons_threshold, 0);
    lv_obj_add_style(button_mode, &style_buttons_threshold_pressed, LV_STATE_PRESSED);
    lv_obj_set_size(button_mode, SIZE_BUTTONS_THRESHOLD, SIZE_BUTTONS_THRESHOLD);
    lv_obj_center(button_mode);

    label_mode = lv_label_create(button_mode);
    lv_label_set_text(label_mode, "M");
    lv_obj_center(label_mode);
	lv_obj_add_event_cb(button_mode, lv_event_handler_button_mode, LV_EVENT_CLICKED, NULL);
    lv_obj_remove_flag(button_mode, LV_OBJ_FLAG_CLICKABLE);


	button_down = lv_button_create(layout_buttons_threshold);
    lv_obj_remove_style_all(button_down);                          /*Remove the style coming from the theme*/
    lv_obj_add_style(button_down, &style_buttons_threshold, 0);
    lv_obj_add_style(button_down, &style_buttons_threshold_pressed, LV_STATE_PRESSED);
    lv_obj_set_size(button_down, SIZE_BUTTONS_THRESHOLD, SIZE_BUTTONS_THRESHOLD);
    lv_obj_center(button_down);

    lv_obj_t * label_down = lv_label_create(button_down);
    lv_label_set_text(label_down, LV_SYMBOL_DOWN);
    lv_obj_center(label_down);

	lv_obj_add_event_cb(button_down, lv_event_handler_button_down, LV_EVENT_CLICKED, &incdec);



}






static void create_layout_buttons_threshold() {


	layout_buttons_threshold = lv_obj_create(screen_main_thermostat);
	set_style_layout_button_threshold();
	lv_obj_add_style(layout_buttons_threshold, &style_layout_buttons_threshold, LV_PART_MAIN);
	lv_obj_set_flex_flow(layout_buttons_threshold, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_pos(layout_buttons_threshold,lv_pct(84) , lv_pct(15));
	lv_obj_set_size(layout_buttons_threshold, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

	//button_up = lv_button_create(layout_buttons_threshold);
	create_buttons_threshold();


}

static void event_handler_button_main_reset(lv_event_t *event) {

    /**
     * Rutina para tratar el reset del dispositivo
     */

    //reset_device();
  
}


static void event_handler_button_factory_reset(lv_event_t *event) {

    /**
     * Rutina para tratar el reset del dispositivo
     */

    //factory_reset_device();
  
}

static void create_button_reset() {

	set_style_reset();
	button_main_reset = lv_button_create(screen_main_thermostat);
	label_main_reset = lv_label_create(button_main_reset);
    lv_obj_add_style(button_main_reset, &style_buttons_threshold, 0);
    lv_obj_add_style(button_main_reset, &style_buttons_threshold_pressed, LV_STATE_PRESSED);
	lv_label_set_text(label_main_reset, LV_SYMBOL_REFRESH);
    lv_obj_set_pos(button_main_reset, lv_pct(1), lv_pct(2));
    lv_obj_set_size(button_main_reset, 55, 55);
    lv_obj_set_style_text_font(label_main_reset, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_center(label_main_reset);
    lv_obj_add_event_cb(button_main_reset, event_handler_button_main_reset, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(button_main_reset, event_handler_button_factory_reset, LV_EVENT_LONG_PRESSED, NULL);



}


void create_layout_schedule() {

    set_style_layout_schedule();
	layout_schedule = lv_obj_create(screen_main_thermostat);
    lv_obj_add_flag(layout_schedule, LV_OBJ_FLAG_HIDDEN);

	lv_obj_set_flex_flow(layout_schedule, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(layout_schedule, 400, 30);
    lv_obj_add_style(layout_schedule, &style_layout_schedule, LV_PART_MAIN);
	text_from_schedule = lv_label_create(layout_schedule);
    progress_schedule = lv_bar_create(layout_schedule);
    lv_obj_set_size(progress_schedule, 250, 15);
    set_style_schedule();
    lv_obj_add_style(progress_schedule, &style_schedule, LV_PART_INDICATOR);
	text_to_schedule = lv_label_create(layout_schedule);
    lv_bar_set_range(progress_schedule, 0, 100);
 	lv_label_set_text(text_from_schedule, "07:00");
	lv_label_set_text(text_to_schedule, "18:50");
    lv_obj_set_style_text_font(text_from_schedule, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_font(text_to_schedule, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_pos(layout_schedule, lv_pct(8), lv_pct(86));

    lv_obj_t *container = lv_obj_create(screen_main_thermostat);
    //lv_obj_align_to(container, layout_schedule, LV_ALIGN_OUT_TOP_MID, -0, 80);
    lv_obj_set_pos(container, lv_pct(15), lv_pct(77));
    lv_obj_set_size(container, 260, 20);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    label_percent = lv_label_create(container);
    lv_obj_center(label_percent);
    lv_obj_add_flag(label_percent, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_font(label_percent, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_add_style(container, &style_layout_schedule, LV_PART_MAIN);
/*
    label_percent = lv_label_create(screen_main_thermostat);

    lv_obj_set_pos(label_percent, 20, 50);
    lv_obj_align_to(label_percent, layout_schedule, LV_ALIGN_OUT_TOP_MID, -40, 10);
    lv_obj_add_flag(label_percent, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_font(label_percent, &lv_font_montserrat_16, LV_PART_MAIN);
*/    

    



}



static void create_label_text_mode() {

    lv_obj_t *layout_text_mode = lv_obj_create(screen_main_thermostat);

    lv_obj_set_pos(layout_text_mode, lv_pct(25), lv_pct(25));
    lv_obj_set_size(layout_text_mode, 180, 20);
    label_text_mode = lv_label_create(layout_text_mode);
    lv_obj_center(label_text_mode);
    set_style_mode();
    lv_obj_add_style(layout_text_mode, &style_text_mode, LV_PART_MAIN);
    lv_obj_remove_flag(layout_text_mode, LV_OBJ_FLAG_SCROLLABLE);


}

void create_warning_icon() {

    
    icon_warning = lv_image_create(screen_main_thermostat);
    lv_image_set_src(icon_warning, &ic_warning);
    lv_obj_set_pos(icon_warning, lv_pct(4), lv_pct(30));
    lv_obj_add_flag(icon_warning, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(icon_warning, lv_event_handler_button_warning, LV_EVENT_CLICKED, NULL);

}


static void lv_event_handler_button_instalation(lv_event_t *e) {

    //remove_task_thermostat();

    //create_factory_screen();
}


void create_instalation_button() {

    icon_instalation = lv_button_create(screen_main_thermostat);
    lv_obj_t *label_instalation = lv_label_create(icon_instalation);
    lv_label_set_text(label_instalation, LV_SYMBOL_SETTINGS);
    lv_obj_set_size(icon_instalation, 40, 40);
    lv_obj_add_style(icon_instalation, &style_buttons_threshold, 0);
    lv_obj_add_style(icon_instalation, &style_buttons_threshold_pressed, LV_STATE_PRESSED);
    lv_obj_set_style_text_font(label_instalation, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_center(label_instalation);
    lv_obj_align_to(icon_instalation, icon_warning, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_obj_add_event_cb(icon_instalation, lv_event_handler_button_instalation, LV_EVENT_CLICKED, NULL);


}

static void init_main_screen() {

    screen_main_thermostat = lv_screen_active();
   //init_fonts();
    font_normal = LV_FONT_DEFAULT;
    screen_main_thermostat = lv_display_get_screen_active(display);
    lv_obj_set_style_text_font(lv_screen_active(), font_normal, 0);

	lv_style_init(&style_screen_init_thermostat);
	lv_theme_default_init(lv_obj_get_disp(lv_screen_active()),  /*Use the DPI, size, etc from this display*/
					lv_color_hex(0x0534F0), lv_color_hex(0x0534F0),   /*Primary and secondary palette*/
			                                        true,    /*Light or dark mode*/
			                                        LV_FONT_DEFAULT); /*Small, normal, large fonts*/

	lv_obj_add_style(screen_main_thermostat, &style_screen_init_thermostat, LV_PART_MAIN);


}


static void create_text_version() {

    lv_obj_t *label_version;
    label_version = lv_label_create(screen_main_thermostat);
    const esp_app_desc_t *app_desc = esp_app_get_description();

    lv_label_set_text_fmt(label_version, "v%s", app_desc->version);
    lv_obj_set_pos(label_version, lv_pct(90),lv_pct(91.5));


}



void lv_create_device_name() {
    
    lv_obj_t *lv_name_device = lv_label_create(screen_main_thermostat);
    lv_obj_set_pos(lv_name_device,70,5);
    //lv_obj_align_to(lv_name_device, button_main_reset, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    //lv_label_set_text(lv_name_device, get_device_name());
}


void create_main_thermostat() {

    init_main_screen();
    create_text_version();
    //screen_main_thermostat = lv_obj_create(disp);
    lv_screen_load(screen_main_thermostat);
    set_style_layout_notification();
    
    create_layout_notification();
/*    create_layout_temperature();
    create_layout_threshold();
	create_heating_icon();
	create_layout_buttons_threshold();
	create_button_reset();
    create_warning_icon();
    lv_update_time(-1,-1);
    lv_update_wifi_status(false);
    lv_update_broker_status(false);



    create_layout_schedule();
    create_label_text_mode();
    */



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
    ESP_LOGI(TAG, "A pintar en display: %.1f como caracter %s", temperature, data);
    lv_label_set_text_fmt(text_temperature, "%s ºC", data);

}

void lv_update_wifi_status(bool status) {

    if (status == true) {

        lv_obj_set_style_text_color(icon_wifi, lv_color_hex(LV_COLOR_TEXT_NOTIFICATION), LV_PART_MAIN);

    } else {
        lv_obj_set_style_text_color(icon_wifi, lv_color_hex(LV_COLOR_TEXT_FAIL_NOTIFICATION), LV_PART_MAIN);
    }
}

void lv_update_broker_status(bool status) {

    ESP_LOGI(TAG, "Cambiamos el broker a %d", status);
    if (status == true) {
        lv_obj_set_style_text_color(icon_broker, lv_color_hex(LV_COLOR_TEXT_NOTIFICATION), LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_color(icon_broker, lv_color_hex(LV_COLOR_TEXT_FAIL_NOTIFICATION), LV_PART_MAIN);
    }

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


void lv_update_schedule(bool show, int max, int index) {

    int cursor;
    time_t now;
    static int inicio = 0;
    struct tm date;
    static int index_schedule;
    static int max_schedule;

    ESP_LOGW(TAG,"lv_update_schedule index: %d, max:%d", index, max );

    if (lv_obj_has_flag(label_percent, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(label_percent, LV_OBJ_FLAG_HIDDEN);
    }

    //Programa aun no valido

    if (index == -1) {
        inicio = time(&now);
        cursor = inicio;
        index_schedule = index;
        //ESP_LOGE(TAG, "Primera vez. Fijamos el inicio a %d", inicio);
        return;
    }

    // programa recien programado
    if (index_schedule == -1) {
        index_schedule = index;
        max_schedule = max;
        //cursor = time(&now) - inicio;
        ESP_LOGW(TAG, "Schedule activado en el display. El nuevo index es %d, con max %d", index_schedule, max);
    } else {
        // El tiempo ha cambiado pero el programa es el mismo
        if ((index_schedule == index) && (max_schedule != max)) {
            max_schedule = max;
            ESP_LOGW(TAG, "Ha cambiado la duracion del programa pero sigue siendo el mismo. El trigger nuevo es : %d", max);

        } else {
            //ha cambiado el programa
            if ((index_schedule != index) || (max_schedule != max)) {
                inicio = time(&now);
                index_schedule = index;
                max_schedule = max;
                ESP_LOGW(TAG, "Hemos cambiado de programa. El nuevo index es %d, con max %d", index_schedule, max);
                }

        }


    }

    //Se comprueba si es el mismo programa o se ha cambiado.

    cursor = time(&now) - inicio;
    
    now = inicio;

    localtime_r(&now, &date);
    lv_label_set_text_fmt(text_from_schedule, "%02d:%02d", date.tm_hour, date.tm_min);
    now = max;
    localtime_r(&now, &date);
    
    lv_label_set_text_fmt(text_to_schedule, "%02d:%02d", date.tm_hour, date.tm_min);




    max = max - inicio;
    
    ESP_LOGW(TAG, "index: %d, max: %d, puntero: %d", index, max, cursor);

    cursor = (cursor * 100)/max;
    ESP_LOGW(TAG, "index: %d, max: %d, puntero en %%: %d", index, max, cursor);
   

    if (show) {
        if (lv_obj_has_flag(layout_schedule, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_remove_flag(layout_schedule, LV_OBJ_FLAG_HIDDEN);
            //lv_bar_set_range(progress_schedule, 0, 100);
            //lv_bar_set_value(progress_schedule, cursor, LV_ANIM_OFF);
        }
        
        lv_bar_set_range(progress_schedule, 0, 100);
        lv_bar_set_value(progress_schedule, cursor, LV_ANIM_OFF);


    } else {
        if (!lv_obj_has_flag(layout_schedule, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(layout_schedule, LV_OBJ_FLAG_HIDDEN);
        }
        
    }


}

void lv_update_icon_errors(bool errors) {

    if (errors == false) {

        lv_obj_add_flag(icon_warning, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(icon_warning, LV_OBJ_FLAG_HIDDEN);
    }


}

void lv_update_text_schedule(int min, int max) {

    lv_label_set_text_fmt(text_from_schedule, "      %d", min);
    lv_label_set_text_fmt(text_to_schedule, "%d   ", max);
    lv_obj_remove_flag(label_percent, LV_OBJ_FLAG_HIDDEN);
}

void lv_update_percent(int cursor) {

    lv_bar_set_value(progress_schedule, cursor, LV_PART_MAIN);
    if (!lv_obj_has_flag(label_percent, LV_OBJ_FLAG_HIDDEN)) {
        lv_label_set_text_fmt(label_percent, "%d%%", cursor);
    }
}



void lv_update_valid_time(bool timevalid) {




}

void lv_upgrade_firmware(char* message, int cursor) {

    if (cursor == -1) {
        lv_obj_add_flag(label_percent, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (lv_obj_has_flag(layout_schedule, LV_OBJ_FLAG_HIDDEN)) {

            lv_obj_remove_flag(layout_schedule, LV_OBJ_FLAG_HIDDEN);
        }

        lv_update_text_schedule(0,100);
        if (lv_obj_has_flag(label_percent, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_remove_flag(label_percent, LV_OBJ_FLAG_HIDDEN);
        } 
        lv_obj_set_size(progress_schedule, 250, 15);
        lv_bar_set_value(progress_schedule, cursor, LV_PART_MAIN);
        
        lv_label_set_text_fmt(label_percent, "%s %d %%", message, cursor);
    }



}

void lv_enable_button_mode(bool enable) {

    if (enable) {
        lv_obj_add_flag(button_mode, LV_OBJ_FLAG_CLICKABLE);
    } else {
        lv_obj_remove_flag(button_mode, LV_OBJ_FLAG_CLICKABLE);

    }
}



void lv_set_button_instalation(bool show) {

    if (show) {

        lv_obj_remove_flag(icon_instalation, LV_OBJ_FLAG_HIDDEN);
        
    } else {
        lv_obj_add_flag(icon_instalation, LV_OBJ_FLAG_HIDDEN);
        
    }
}


