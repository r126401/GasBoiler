idf_component_register(SRCS 
"app_interface.c"
INCLUDE_DIRS 
"include"
#REQUIRES nvs_flash esp_timer thermostat_task rgb_task
PRIV_REQUIRES lcdlvgl thermostat_task rainmaker_interface


)

