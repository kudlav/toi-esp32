#include "main.h"

void adc1Init() {
	// Configure ADC1 capture width12 bit
	adc1_config_width(ADC_WIDTH_BIT_12);
	// Init light sensor channel GPIO34
	adc1_config_channel_atten(CHANNEL_LIGHT, ADC_ATTEN_DB_11);
}

int lightGet() {
	int val = adc1_get_raw(CHANNEL_LIGHT);
	ESP_LOGI("lightGet", "Light sensor readings (0-4095): %d", val);
	return val;
}

int hallGet() {
	int val = hall_sensor_read();
	ESP_LOGI("hallGet", "Hall sensor readings (0-4095): %d", val);
	return val;
}

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());

	wifi_init_sta();
	adc1Init();

	bool tempOk = (tempInit() > 0);
	bool coapOk = coapInit();

	while(tempOk && coapOk) {
        char data[512];

		// Get values
		float temp = getDsTemp(0);
		int light = lightGet();
		int hall = hallGet();

		// Get current UTC time
		time_t t = time(NULL);
		struct tm *ptm = gmtime(&t);

        sprintf(
			data,
			"time=%d-%02d-%02d %02d:%02d:%02d;tmpRaw=%f;tmpKlm=TODO;lux=%d;hal=%d;cpu=TODO;dev=%s",
			ptm->tm_year + 1900,
			ptm->tm_mon,
			ptm->tm_mday,
			ptm->tm_hour%24,
			ptm->tm_min,
			ptm->tm_sec,
			temp,
			light,
			hall,
			DEVICE_NAME
		);

		ESP_LOGI("main", "Collected data to send: %s", data);

		coapSend((unsigned char *) data);

		vTaskDelay(5000.0 / portTICK_PERIOD_MS);
    }

	coapCleanup();
}
