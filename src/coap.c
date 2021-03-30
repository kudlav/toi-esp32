/* CoAP sender
 * This code is based on Public Domain example:
 * https://github.com/espressif/esp-idf/blob/master/examples/protocols/coap_client/main/coap_client_example_main.c
*/

#include "coap.h"

const static char *TAG = "CoAP_client";

/* Coap client state */
static coap_optlist_t *optlist = NULL;
static coap_session_t *session = NULL;
static coap_context_t *ctx = NULL;

bool coapInit() {

	struct hostent *hp;
	coap_address_t    dst_addr;
	static coap_uri_t uri;
	const char       *server_uri = COAP_URI;
	char *phostname = NULL;

	coap_set_log_level(LOG_DEBUG);

	unsigned char _buf[BUFSIZE];
	unsigned char *buf;
	size_t buflen;
	int res;

	optlist = NULL;
	if (coap_split_uri((const uint8_t *)server_uri, strlen(server_uri), &uri) == -1) {
		ESP_LOGE(TAG, "CoAP server uri error");
		return false;
	}

	if (uri.scheme == COAP_URI_SCHEME_COAPS && !coap_dtls_is_supported()) {
		ESP_LOGE(TAG, "MbedTLS (D)TLS Client Mode not configured");
		return false;
	}
	if (uri.scheme == COAP_URI_SCHEME_COAPS_TCP && !coap_tls_is_supported()) {
		ESP_LOGE(TAG, "CoAP server uri coaps+tcp:// scheme is not supported");
		return false;
	}

	phostname = (char *)calloc(1, uri.host.length + 1);
	if (phostname == NULL) {
		ESP_LOGE(TAG, "calloc failed");
		return false;
	}

	memcpy(phostname, uri.host.s, uri.host.length);
	hp = gethostbyname(phostname);
	free(phostname);

	if (hp == NULL) {
		ESP_LOGE(TAG, "DNS lookup failed");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		free(phostname);
		return false;
	}
	char tmpbuf[INET6_ADDRSTRLEN];
	coap_address_init(&dst_addr);
	switch (hp->h_addrtype) {
		case AF_INET:
			dst_addr.addr.sin.sin_family      = AF_INET;
			dst_addr.addr.sin.sin_port        = htons(uri.port);
			memcpy(&dst_addr.addr.sin.sin_addr, hp->h_addr, sizeof(dst_addr.addr.sin.sin_addr));
			inet_ntop(AF_INET, &dst_addr.addr.sin.sin_addr, tmpbuf, sizeof(tmpbuf));
			ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", tmpbuf);
			break;
		case AF_INET6:
			dst_addr.addr.sin6.sin6_family      = AF_INET6;
			dst_addr.addr.sin6.sin6_port        = htons(uri.port);
			memcpy(&dst_addr.addr.sin6.sin6_addr, hp->h_addr, sizeof(dst_addr.addr.sin6.sin6_addr));
			inet_ntop(AF_INET6, &dst_addr.addr.sin6.sin6_addr, tmpbuf, sizeof(tmpbuf));
			ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", tmpbuf);
			break;
		default:
			ESP_LOGE(TAG, "DNS lookup response failed");
			coapCleanup();
			return false;
	}

	if (uri.path.length) {
		buflen = BUFSIZE;
		buf = _buf;
		res = coap_split_path(uri.path.s, uri.path.length, buf, &buflen);

		while (res--) {
			coap_insert_optlist(&optlist,
								coap_new_optlist(COAP_OPTION_URI_PATH,
													coap_opt_length(buf),
													coap_opt_value(buf)));

			buf += coap_opt_size(buf);
		}
	}

	if (uri.query.length) {
		buflen = BUFSIZE;
		buf = _buf;
		res = coap_split_query(uri.query.s, uri.query.length, buf, &buflen);

		while (res--) {
			coap_insert_optlist(&optlist,
								coap_new_optlist(COAP_OPTION_URI_QUERY,
													coap_opt_length(buf),
													coap_opt_value(buf)));

			buf += coap_opt_size(buf);
		}
	}

	ctx = coap_new_context(NULL);
	if (!ctx) {
		ESP_LOGE(TAG, "coap_new_context() failed");
		coapCleanup();
		return false;
	}

	/*
		* Note that if the URI starts with just coap:// (not coaps://) the
		* session will still be plain text.
		*
		* coaps+tcp:// is NOT supported by the libcoap->mbedtls interface
		* so COAP_URI_SCHEME_COAPS_TCP will have failed in a test above,
		* but the code is left in for completeness.
		*/
	if (uri.scheme == COAP_URI_SCHEME_COAPS || uri.scheme == COAP_URI_SCHEME_COAPS_TCP) {
		session = coap_new_client_session_psk(ctx, NULL, &dst_addr,
												uri.scheme == COAP_URI_SCHEME_COAPS ? COAP_PROTO_DTLS : COAP_PROTO_TLS,
												EXAMPLE_COAP_PSK_IDENTITY,
												(const uint8_t *)EXAMPLE_COAP_PSK_KEY,
												sizeof(EXAMPLE_COAP_PSK_KEY) - 1);
	} else {
		session = coap_new_client_session(ctx, NULL, &dst_addr,
											uri.scheme == COAP_URI_SCHEME_COAP_TCP ? COAP_PROTO_TCP :
											COAP_PROTO_UDP);
	}
	if (!session) {
		ESP_LOGE(TAG, "coap_new_client_session() failed");
		coapCleanup();
		return false;
	}

	return true;
}

bool coapSend(unsigned char data[]) {

	coap_pdu_t *request = coap_new_pdu(session);
	if (!request) {
		ESP_LOGE(TAG, "coap_new_pdu() failed");
		coapCleanup();
		return false;
	}
	request->type = COAP_MESSAGE_CON;
	request->code = COAP_REQUEST_POST;
	request->tid = coap_new_message_id(session);
	coap_add_optlist_pdu(request, &optlist);
	coap_add_data(request, strlen((char*)data), data);

	coap_send(session, request);

	int wait_ms = COAP_DEFAULT_TIME_SEC * 1000;

	while (1) {
		int result = coap_run_once(ctx, wait_ms > 1000 ? 1000 : wait_ms);
		if (result >= 0) {
			if (result >= wait_ms) {
				break;
			} else {
				wait_ms -= result;
			}
		} else {
			ESP_LOGE(TAG, "coap_run_once error");
		}
	}

	return true;
}

void coapCleanup() {
	if (optlist) {
		coap_delete_optlist(optlist);
		optlist = NULL;
	}
	if (session) {
		coap_session_release(session);
	}
	if (ctx) {
		coap_free_context(ctx);
	}
	coap_cleanup();
}
