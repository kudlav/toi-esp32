#ifndef TOI_PROCESSING_TASK_H
#define TOI_PROCESSING_TASK_H

#include "esp_log.h"
#include <math.h> // fabs()
#include "esp_timer.h" // esp_timer_get_time()

#include "main.h"

#define KALMAN_VAR 0.01
#define KALMAN_ERR 1

void processingTask(void *pvParameter);

#endif
