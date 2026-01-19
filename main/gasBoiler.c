#include <stdio.h>

#include "rgblcd.h"


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "GasBoiler.h"
#include "thermostat_task.h"
#include "rainmaker_interface.h"
#include "app_interface.h"

EventGroupHandle_t evt_between_task;


void app_main(void)
{

    evt_between_task = xEventGroupCreate();
    init_lcdrgb();
    create_task_thermostat();
    init_app_environment();
    set_status_app(STATUS_APP_STARTING);


    




}
