#ifndef TOI_COAP_H
#define TOI_COAP_H

#include "libcoap.h"

#include "address.h"
#include "async.h"
#include "bits.h"
#include "block.h"
#include "coap_dtls.h"
#include "coap_event.h"
#include "coap_io.h"
#include "coap_time.h"
#include "coap_debug.h"
#include "encode.h"
#include "mem.h"
#include "net.h"
#include "option.h"
#include "pdu.h"
#include "prng.h"
#include "resource.h"
#include "str.h"
#include "subscribe.h"
#include "uri.h"

#include "esp_log.h"
#include <lwip/netdb.h>
#include <string.h>

bool coapInit();
bool coapSend(unsigned char data[]);
void coapCleanup();

#endif
