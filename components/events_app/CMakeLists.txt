idf_component_register(SRCS 
"events_app.c" "alarms_app.c"
INCLUDE_DIRS 
"include"
#REQUIRES nvs_flash esp_timer thermostat_task rgb_task
PRIV_REQUIRES lcdlvgl thermostat_task


)

