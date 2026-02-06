idf_component_register(SRCS 
"lv_main.c"
"styles.c"
"events_lcd.c"
"russo36.c" 
"russo48.c"
"ic_heating.c" 
"ic_thermometer.c" 
"ic_threshold.c" 
"ic_warning.c"
"rgblcd.c"
INCLUDE_DIRS 
"include" 
PRIV_REQUIRES lvgl__lvgl espressif__esp_lcd_touch atanisoft__esp_lcd_touch_xpt2046 esp_timer espressif__qrcode rainmaker_interface app_interface json

)

