#include "main.h"

//static const char TAG[] = "main";

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());

	// APP_CPU
	xTaskCreatePinnedToCore(&sensorsTask, "sensorsTask", 4096, NULL, 5, NULL, 1);
	// PRO_CPU
	xTaskCreatePinnedToCore(&processingTask, "processingTask", 2048, NULL, 5, NULL, 0);
}
