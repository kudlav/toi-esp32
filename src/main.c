#include "main.h"

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());

	xTaskCreate(&onewire_task, "onewire_task", 2048, NULL, 5, NULL);
	
	wifi_init_sta();
	
	send_data();
}
