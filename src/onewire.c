#include "onewire.h"

static int numDevices = 0;
static OneWireBus *owb = NULL;
static DS18B20_Info *devices[MAX_DEVICES];

static const char TAG[] = "one-wire";

int tempInit() {

	if (numDevices > 0) {
		ESP_LOGE(TAG, "Temp sensors already initiated, found: %d", numDevices);
		return numDevices;
	}

	// Stable readings require a brief period before communication
	vTaskDelay(2000.0 / portTICK_PERIOD_MS);

	// Create a 1-Wire bus, using the RMT timeslot driver
	owb_rmt_driver_info rmt_driver_info;
	owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0,
							 RMT_CHANNEL_1, RMT_CHANNEL_0);
	owb_use_crc(owb, true);  // enable CRC check for ROM code

	// Find all connected devices
	ESP_LOGI(TAG, "Find devices:\n");
	OneWireBus_ROMCode device_rom_codes[MAX_DEVICES] = {0};
	numDevices = 0;
	OneWireBus_SearchState search_state = {0};
	bool found = false;
	owb_search_first(owb, &search_state, &found);
	while (found)
	{
		char rom_code_s[17];
		owb_string_from_rom_code(search_state.rom_code, rom_code_s,
								 sizeof(rom_code_s));
		printf("  %d : %s\n", numDevices, rom_code_s);
		device_rom_codes[numDevices] = search_state.rom_code;
		++numDevices;
		owb_search_next(owb, &search_state, &found);
	}
	ESP_LOGI(TAG, "Found %d device%s", numDevices,
						 numDevices == 1 ? "" : "s");

	// Create DS18B20 devices on the 1-Wire bus
	for (int i = 0; i < numDevices; ++i)
	{
		DS18B20_Info * ds18b20_info = ds18b20_malloc();
		// heap allocation
		devices[i] = ds18b20_info;

		if (numDevices == 1) {
			ESP_LOGI(TAG, "Single device optimisations enabled");
			ds18b20_init_solo(ds18b20_info, owb);
			// only one device on bus
		} else {
			ds18b20_init(ds18b20_info, owb, device_rom_codes[i]);
			// associate with bus and device
		}
		ds18b20_use_crc(ds18b20_info, true);
		// enable CRC check on all reads
		ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);
	}
	return numDevices;
}

float getDsTemp(int deviceId) {
ESP_LOGI(TAG, "num devices: %d", numDevices);
	if (deviceId >= numDevices) {
		ESP_LOGE(TAG, "Requested device #%d but evailable devices: %d", deviceId, numDevices);
		return NAN;
	}

	// Read temperatures from all sensors sequentially
	ESP_LOGI(TAG, "Temperature readings (degrees C):");
/*	ds18b20_convert_all(owb);

	// In this application all devices use the same resolution,
	// so use the first device to determine the delay
	ds18b20_wait_for_conversion(devices[0]);

	float temp = 0;
	ds18b20_read_temp(devices[deviceId], &temp);
	ESP_LOGI(TAG, "  %d: %.3f\n", deviceId, temp);
	return temp;
*/
	return NAN;
}
