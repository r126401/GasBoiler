#include "stdbool.h"

typedef enum STATUS_GAS_BOILER {

    STATUS_APP_ERROR = -1, 
    STATUS_APP_AUTO, 
    STATUS_APP_MANUAL, 
    STATUS_APP_STARTING, 
    STATUS_APP_CONNECTING, 
    STATUS_APP_UPGRADING, 
    STATUS_APP_UNKNOWN


}STATUS_GAS_BOILER;



#define DEFAULT_TEMPERATURE_CORRECTION -3.5
#define DEFAULT_ALARM "NO ALARM"
#define DEFAULT_READ_INTERVAL 60
#define INVALID_TEMPERATURE -100


void rainmaker_interface_init_environment();
float get_setpoint_temperature();
void notify_current_temperature(float current_temperature);
void notify_sensor_fail();
int get_read_interval();
STATUS_GAS_BOILER get_status_gas_boiler();
float get_current_temperature();