#ifndef TOI_MAIN_H
#define TOI_MAIN_H

#include "esp_system.h" // ESP_ERROR_CHECK()
#include "nvs_flash.h" // nvs_flash_init()
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "sensorsTask.h"
#include "processingTask.h"
#include "idleTask.h"

void app_main();
QueueHandle_t toSensorsQueue;
QueueHandle_t toProcessingQueue;

#endif
