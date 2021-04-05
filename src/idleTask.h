#ifndef TOI_IDLE_TASK_H
#define TOI_IDLE_TASK_H

#include "esp_timer.h" // esp_timer_get_time()
#include "freertos/FreeRTOS.h" // portTICK_RATE_MS
#include "freertos/task.h" // vTaskDelay()

int idleSensTime;
int idleProcTime;

void idleSensors(void *pvParameter);
void idleProcessing(void *pvParameter);

#endif
