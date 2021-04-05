/*
 * ESP32 CPU load
 * based on https://github.com/pglen/esp32_cpu_load
 * Public Domain or CC0
 */

#include "idleTask.h"

int idleSensTime = 0;
int idleProcTime = 0;

void idleSensors(void *pvParameter) {
	while (true) {
		int64_t now = esp_timer_get_time();     // time anchor
		vTaskDelay(0.0 / portTICK_PERIOD_MS);
		int64_t now2 = esp_timer_get_time();
		idleSensTime += (now2 - now) / 1000;        // diff [ms]
	}
}

void idleProcessing(void *pvParameter) {
	while (true) {
		int64_t now = esp_timer_get_time();     // time anchor
		vTaskDelay(0.0 / portTICK_PERIOD_MS);
		int64_t now2 = esp_timer_get_time();
		idleProcTime += (now2 - now) / 1000;        // diff [ms]
	}
}
