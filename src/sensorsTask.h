#ifndef TOI_SENSORS_TASK_H
#define TOI_SENSORS_TASK_H

#include <stdio.h>
#include <time.h>
#include "esp_log.h"

#include <driver/adc.h>

#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"
#include <math.h>

#include "wifi.h"
#include "coap.h"

#define CHANNEL_LIGHT ADC1_CHANNEL_6
#define DEVICE_NAME "espXKUDLA15" // DEVICE NAME

#define GPIO_DS18B20_0       (4)
#define DS18B20_RESOLUTION   (DS18B20_RESOLUTION_12_BIT)

void sensorsTask(void *pvParameter);

#endif
