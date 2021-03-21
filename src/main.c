#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/gpio.h"

#include "esp_http_client.h"

#include "freertos/queue.h"

#define GPIO_LED_BOARD  2
#define GPIO_LED_WHITE  13
#define GPIO_LED_GREEN  12
#define GPIO_LED_RED    14

void wifi_init_sta(void);             // deklarace funkce z wifi.c
void onewire_task(void *pvParameter); // deklarace funkce z onewire.c
const QueueHandle_t queue;            // deklarace funkce z onewire.c

void hello_task(void *pvParameter)
{
    printf("Hello world!\n");
    for (int i = 1; 1 ; i++) {
        printf("Running %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void blink_task(void *pvParameter) {
    gpio_pad_select_gpio(GPIO_LED_BOARD);
    gpio_pad_select_gpio(GPIO_LED_WHITE);
    gpio_pad_select_gpio(GPIO_LED_GREEN);
    gpio_pad_select_gpio(GPIO_LED_RED);


    /* Set the GPIO as a push/pull output */
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_BOARD, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_WHITE, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_GREEN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_RED, GPIO_MODE_OUTPUT));

    int cnt = 0;

    while(1) {
        /* Blink off (output low) */
        ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_BOARD, cnt & 0x01));
        ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_WHITE, cnt & 0x01));
        ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_GREEN, cnt & 0x02));
        ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_RED, cnt & 0x04));
        vTaskDelay(500 / portTICK_RATE_MS);
        cnt++;
    }
}

void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
//    xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
//    xTaskCreate(&blink_task, "blink_task", 512, NULL, 5, NULL);

    xTaskCreate(&onewire_task, "onewire_task", 2048, NULL, 5, NULL);
    
    wifi_init_sta();

    gpio_pad_select_gpio(GPIO_LED_WHITE);
    gpio_pad_select_gpio(GPIO_LED_GREEN);
    gpio_pad_select_gpio(GPIO_LED_RED);
    /* Set the GPIO as a push/pull output */
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_WHITE, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_GREEN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_RED, GPIO_MODE_OUTPUT));

    while(1) {
        float temp;
        char url[100];

        xQueueReceive(queue, &temp, portMAX_DELAY);

        sprintf(url, "https://ehw.fit.vutbr.cz/toi/write.php?id=20&temp=%f&note=xkudla15", temp);

        esp_http_client_config_t config = {
            .url = url, // vase url
            .event_handler = NULL // nebudeme zaznamenávat chyby HTTP
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_err_t err = esp_http_client_perform(client);

        if (err == ESP_OK) {
            ESP_LOGI("ethernet", "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));

            char buff[128] = "";
            esp_http_client_read_response(client, buff, 128);
            ESP_LOGI("ethernet", "response = %s", buff);

            ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_WHITE, buff[0] & 0x01));
            ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_GREEN, buff[0] & 0x02));
            ESP_ERROR_CHECK(gpio_set_level(GPIO_LED_RED, buff[0] & 0x04));
        }
        else {
            ESP_LOGE("ethernet", "chyba = %d", err);
        }

        esp_http_client_cleanup(client);
    }
}
