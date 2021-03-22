#ifndef TOI_ONEWIRE_H
#define TOI_ONEWIRE_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

#include "freertos/queue.h"
#include <math.h>

#define GPIO_DS18B20_0       (4)
#define MAX_DEVICES          (8)
#define DS18B20_RESOLUTION   (DS18B20_RESOLUTION_12_BIT)
#define SAMPLE_PERIOD        (1000)   // milliseconds

int tempInit();
float getDsTemp(int);

#endif
