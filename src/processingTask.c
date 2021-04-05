#include "processingTask.h"

/*
 * SimpleKalmanFilter - a Kalman Filter implementation for single variable models.
 * based on https://github.com/denyssene/SimpleKalmanFilter
 * MIT License, Copyright (c) 2017 Denys Sene
 */
float kalmanFilter(float input) {
  	static float expErr = KALMAN_ERR;
	static float lastOut = 0.0;

	float kalmanGain = expErr / (expErr + KALMAN_ERR);
	float currOut = lastOut + kalmanGain * (input - lastOut);
	expErr = (1.0 - kalmanGain) * expErr + fabs(lastOut - currOut) * KALMAN_VAR;
	lastOut = currOut;

	return currOut;
}

void processingTask(void *pvParameter) {
	static const char TAG[] = "processingTask";
	int64_t prevTime = esp_timer_get_time();

	while (true) {
		float rawTemp;
		xQueueReceive(toProcessingQueue, &rawTemp, portMAX_DELAY);
		ESP_LOGI(TAG, "Recieved raw temperature: %.3f", rawTemp);

		float filteredTemp = kalmanFilter(rawTemp);

		int64_t now = esp_timer_get_time();
		float totalTime = (now - prevTime) / 1000.0; // diff [ms]
		prevTime = now;

		int idleProc = idleProcTime;
		int idleSens = idleSensTime;
		idleProcTime = 0;
		idleSensTime = 0;
		float sensorsUtil = 100 - (100 * idleProc / totalTime);
		float processingUtil = 100 - (100 * idleSens / totalTime);

		ESP_LOGI(TAG, "Sending filtered temperature: %.3f", filteredTemp);
		xQueueSend(toSensorsQueue, (void*) &filteredTemp, (TickType_t) 0);
		xQueueSend(toSensorsQueue, (void*) &processingUtil, (TickType_t) 0);
		xQueueSend(toSensorsQueue, (void*) &sensorsUtil, (TickType_t) 0);
	}
}
