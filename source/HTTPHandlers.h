#ifndef __HTTPHandlers_h__
#define __HTTPHandlers_h__

#include "mbed.h"
#include "HttpResponseBuilder.h"

void request_handler(HttpParsedRequest* request, ClientConnection* clientConnection);
void request_handler_getStatus(HttpParsedRequest* request, ClientConnection* clientConnection);

#endif