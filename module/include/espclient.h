/*
 * espclient.h
 *
 *  Created on: Apr 8, 2015
 *      Author: HieuNT | hieucdtspk@gmail.com
 *
 *  Copyright (c) 2015 HieuNT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MODULE_INCLUDE_ESPCLIENT_H_
#define MODULE_INCLUDE_ESPCLIENT_H_

typedef struct espclient espclient;

/*
 * args: pointer to espclient
 */
typedef void (*EspCallback)(uint32_t *args);
/*
 * args: pointer to espclient
 * pdata: pointer to received data
 * len: len of received data
 */
typedef void (*EspDataCallback)(uint32_t *args, uint8_t *pdata, uint16_t len);

typedef enum espclient_security espclient_security_t;

enum espclient_security {
	ESPCLIENT_SEC_NONE = 0,
	ESPCLIENT_SEC_SSL,
};

struct espclient {
	struct espconn *conn;
	uint8_t security;
	uint8_t *host;
	uint32_t port;
	ip_addr_t ip;
	EspCallback connectedCb;
	EspCallback disconnectedCb;
	EspCallback dnsFailedCb;
	EspCallback sentCb;
	EspDataCallback dataCb;
};

/* -------------------------------------------------------------------------------------
 * FUNCTIONS
 * ------------------------------------------------------------------------------------- */
void ICACHE_FLASH_ATTR EspClient_Init(espclient *espClient);
void ICACHE_FLASH_ATTR EspClient_Connect(espclient *espClient, \
		char *host, uint32_t port, espclient_security_t security);
void ICACHE_FLASH_ATTR EspClient_Disconnect(espclient *espClient);
void ICACHE_FLASH_ATTR EspClient_Send(espclient *espClient, uint8_t *pdata, uint16_t len);
void ICACHE_FLASH_ATTR EspClient_OnConnected(espclient *espClient, EspCallback connectedCb);
void ICACHE_FLASH_ATTR EspClient_OnDisconnected(espclient *espClient, EspCallback disconnectedCb);
void ICACHE_FLASH_ATTR EspClient_OnDnsFailed(espclient *espClient, EspCallback dnsFailedCb);
void ICACHE_FLASH_ATTR EspClient_OnSent(espclient *espClient, EspCallback sentCb);
void ICACHE_FLASH_ATTR EspClient_OnData(espclient *espClient, EspDataCallback dataCb);

#endif /* MODULE_INCLUDE_ESPCLIENT_H_ */
