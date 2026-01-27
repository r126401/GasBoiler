

#include <inttypes.h>
#include <stdint.h>

typedef struct schedules_t {

    int index;
    bool enabled;
    int mask_trigger;
    char name[5];
    int trigger;
    float temperature;

} schedules_t;

uint8_t get_schedules_list();
uint8_t get_next_schedule(int *seconds_of_day, int *seconds_of_trigger, float *setpoint_temperature);
int get_data_schedules(int *min_of_day, int *min_of_trigger, float *setpoint_temperature);