idf_component_register(
    SRCS "rainmaker_interface.c" "rainmaker_events.c"
    INCLUDE_DIRS 
    "include" 
    PRIV_REQUIRES espressif__rmaker_common espressif__esp_rainmaker espressif__rmaker_app_network nvs_flash
    #PRIV_REQUIRES events_app espressif__ds18b20 espressif__onewire_bus esp_driver_gpio lcdlvgl espressif__esp_schedule
    
)
