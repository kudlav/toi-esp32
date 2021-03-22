#include "main.h"

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());
	
	wifi_init_sta();

	bool tempOk = (tempInit() > 0);
	bool coapOk = coapInit();
	float temp = getDsTemp(0);
ESP_LOGI("main", "temp: %f", temp);
return;
	while(tempOk && coapOk) {
        char data[512];

		// Get temperature
		float temp = getDsTemp(0);

        sprintf(data, "tmpRaw=%f;tmpFilter=TODO;lux=TODO;hall=TODO;tmpDev=TODO;cpu=TODO;dev=espXKUDLA15", temp);

		coapSend((unsigned char *) data);
break;
		vTaskDelay(5000.0 / portTICK_PERIOD_MS);
    }

	coapCleanup();
}
