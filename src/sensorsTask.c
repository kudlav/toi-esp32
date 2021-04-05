#include "sensorsTask.h"

static const char TAG[] = "sensorsTask";

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

void sensorsTask(void *pvParameter) {

	wifi_init_sta();
	adc1Init();

	bool coapOk = coapInit();
	bool tempOk = false;

	// TEMP INIT
	ESP_LOGI(TAG, "Finding DS18B20 device");
	OneWireBus *owb = NULL;
	DS18B20_Info *device = NULL;
	// Stable readings require a brief period before communication
	vTaskDelay(2000.0 / portTICK_PERIOD_MS);
	// Create a 1-Wire bus, using the RMT timeslot driver
	owb_rmt_driver_info rmt_driver_info;
	owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0, RMT_CHANNEL_1, RMT_CHANNEL_0);
	owb_use_crc(owb, true);  // enable CRC check for ROM code
	// Find connected device
	OneWireBus_SearchState search_state = {0};
	owb_search_first(owb, &search_state, &tempOk);
	if (tempOk) {
		char rom_code_s[17];
		owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
		printf("%s\n", rom_code_s);
		ESP_LOGI(TAG, "Device found");
		// Create DS18B20 device on the 1-Wire bus
		DS18B20_Info * ds18b20_info = ds18b20_malloc();
		// heap allocation
		device = ds18b20_info;
		// Single device optimisations enabled
		ds18b20_init_solo(ds18b20_info, owb);
		ds18b20_use_crc(ds18b20_info, true);
		// enable CRC check on all reads
		ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);
	}

	while(tempOk && coapOk) {
        char data[512];

		// Get values
		int light = lightGet();
		int hall = hallGet();

		// Read temperature
		float temp = 0;
		ds18b20_convert_all(owb);
		ds18b20_wait_for_conversion(device);
		ds18b20_read_temp(device, &temp);
		ESP_LOGI(TAG, "Temperature readings (degrees C): %.3f", temp);

		// Get current UTC time
		time_t t = time(NULL);
		struct tm *ptm = gmtime(&t);

        sprintf(
			data,
			"time=%d-%02d-%02d %02d:%02d:%02d;tmpRaw=%.3f;tmpKlm=TODO;lux=%d;hal=%d;cpu=TODO;dev=%s",
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

		vTaskDelay(2000.0 / portTICK_PERIOD_MS);
    }

	coapCleanup();

}
