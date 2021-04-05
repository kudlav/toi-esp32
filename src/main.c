#include "main.h"

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());

	toSensorsQueue = xQueueCreate(3, sizeof(float)); // pass filteredTemp and cpu0, cpu1 utilization
	toProcessingQueue = xQueueCreate(1, sizeof(float)); // pass rawTemp

	// APP_CPU
	xTaskCreatePinnedToCore(&idleSensors, "idleSensors", 2048, NULL, 0, NULL, 1); // priority 0 (low)
	xTaskCreatePinnedToCore(&sensorsTask, "sensorsTask", 4096, NULL, 5, NULL, 1); // priority 5
	// PRO_CPU
	xTaskCreatePinnedToCore(&idleProcessing, "idleProcessing", 2048, NULL, 0, NULL, 0); // priority 0 (low)
	xTaskCreatePinnedToCore(&processingTask, "processingTask", 2048, NULL, 5, NULL, 0); // priority 5
}
