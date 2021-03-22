#ifndef TOI_MAIN_H
#define TOI_MAIN_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/gpio.h"

#include "esp_http_client.h"

#include "freertos/queue.h"

#include "wifi.h"
#include "onewire.h"

#define GPIO_LED_BOARD  2
#define GPIO_LED_WHITE  13
#define GPIO_LED_GREEN  12
#define GPIO_LED_RED    14

#endif
