idf_component_register(
    SRCS "rainmaker_interface.c" "rainmaker_schedule.c"
    INCLUDE_DIRS 
    "include" 
    PRIV_REQUIRES espressif__rmaker_common espressif__esp_rainmaker espressif__rmaker_app_network nvs_flash lcdlvgl esp_wifi esp_timer app_interface json json_generator
    
)
