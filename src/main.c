#include "main.h"

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());

	xTaskCreate(&onewire_task, "onewire_task", 2048, NULL, 5, NULL);
	
	wifi_init_sta();

	bool coapOk = coapInit();

	while(coapOk) {
        float temp;
        char data[512];

        xQueueReceive(queue, &temp, portMAX_DELAY);

        sprintf(data, "tmpRaw=%f;tmpFilter=TODO;lux=TODO;hall=TODO;tmpDev=TODO;cpu=TODO;dev=espXKUDLA15", temp);

		coapSend((unsigned char *) data);
    }

	coapCleanup();
}
