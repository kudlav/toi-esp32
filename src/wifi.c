#include "wifi.h"

/* https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c */

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all 
             * security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be 
             * used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting 
             * below line */
         .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

bool send_data() {
	const char TAG[] = "send_data";

	/* DNS lookup for host address */
	struct hostent *server = gethostbyname(host);
	if (server == NULL) {
		ESP_LOGE(TAG, "Unable to find server: %s", host);
		return false;
	}

	int addr_family = 0;
	int ip_protocol = 0;
	struct sockaddr* dest_addr;
	
	switch (server->h_addrtype) {
		case AF_INET: {
			struct sockaddr_in dest_addr4;
			dest_addr4.sin_addr.s_addr = inet_addr(server->h_addr_list[0]);
			dest_addr4.sin_family = AF_INET;
			dest_addr4.sin_port = htons(port);
			addr_family = AF_INET;
			ip_protocol = IPPROTO_IP;
			dest_addr = (struct sockaddr *) &dest_addr4;
			break;
		}
		case AF_INET6: {
			struct sockaddr_in6 dest_addr6 = { 0 };
			inet6_aton(server->h_addr_list[0], &dest_addr6.sin6_addr);
			dest_addr6.sin6_family = AF_INET6;
			dest_addr6.sin6_port = htons(port);
			dest_addr6.sin6_scope_id = esp_netif_get_netif_impl_index(WIFI_IF_STA);
			addr_family = AF_INET6;
			ip_protocol = IPPROTO_IPV6;
			dest_addr = (struct sockaddr *) &dest_addr6;
			break;
		}
		default:
			ESP_LOGE(TAG, "DNS lookup response failed: %s", host);
			return false;
	}

	/* Create socket */
	int client_sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
	if (client_sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket, errno: %d", errno);
		return false;
	}
	ESP_LOGI(TAG, "Socket created, sending to %s:%d", host, port);

	const char payload[] = "FUU BAR";

	/* Send message */
	int err = sendto(client_sock, payload, strlen(payload), 0, dest_addr, sizeof(*dest_addr));
	if (err < 0) {
		ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
		close(client_sock);
		return false;
	}
	ESP_LOGI(TAG, "Message sent");

	return true;

}
