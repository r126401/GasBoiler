


#define READ_INTERVAL 60;






typedef enum THERMOSTAT_ACTION {
	NO_TOGGLE_THERMOSTAT,
	TOGGLE_THERMOSTAT,
   NO_ACTION_THERMOSTAT
}THERMOSTAT_ACTION;

typedef enum DEVICE_STATUS {

   DEVICE_OK,
   SENSOR_FAIL
} DEVICE_STATUS;

typedef enum THERMOSTAT_MODE {

    MODE_AUTO,
    MODE_AUTOMAN,
    MODE_MANUAL,
    MODE_ERROR

} THERMOSTAT_MODE;

void create_task_thermostat();
THERMOSTAT_ACTION thermostat_action(float current_temperature);
void remove_task_thermostat();


