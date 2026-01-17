idf_component_register(
    SRCS "thermostat_task.c"
    INCLUDE_DIRS 
    "include" 
    PRIV_REQUIRES espressif__ds18b20 espressif__onewire_bus esp_driver_gpio lcdlvgl app_interface
    
)
