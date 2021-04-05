#include "processingTask.h"

void processingTask(void *pvParameter) {

	while (true) {
		vTaskDelay(2000.0 / portTICK_PERIOD_MS);
	}

}
