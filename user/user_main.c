/*
 * user_main.c --- main entry for espclient test!
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

#include "esp_common.h"
#include "uart.h"
#include "led.h"
#include "wifi.h"
#include "espclient.h"
#include "debug.h"

void ICACHE_FLASH_ATTR espclient_test_init(void);

void ICACHE_FLASH_ATTR userWifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
		INFO("Wifi connected!\n\r");
		espclient_test_init();
	} else {
		INFO("Wifi disconnected!\n\r");
	}
}

/*
 * ESPCLIENT test
 */
void ICACHE_FLASH_ATTR espclient_test_connected_cb(uint32_t *args)
{
	espclient *espClient = (espclient *)args;
	uint8_t welcome[] = "Hello, this is ESP8266 ESPCLIENT\n\r";

	INFO("EspClient connected!\n\r");
	EspClient_Send(espClient, welcome, sizeof(welcome));
}
void ICACHE_FLASH_ATTR espclient_test_disconnected_cb(uint32_t *args)
{
	INFO("EspClient disconnected!\n\r");
}
void ICACHE_FLASH_ATTR espclient_test_dnsfailed_cb(uint32_t *args)
{
	INFO("EspClient dns failed!\n\r");
}
void ICACHE_FLASH_ATTR espclient_test_sent_cb(uint32_t *args)
{
	INFO("EspClient sent!\n\r");
}
void ICACHE_FLASH_ATTR espclient_test_data_cb(uint32_t *args, uint8_t *pdata, uint16_t len)
{
	char ok[] = "OK!";

	INFO("EspClient data!\n\r");

	INFO("Len: %u\n\r", len);
	INFO("Data: %s\n\r", pdata);

	espclient *espClient = (espclient *)args;
	EspClient_Send(espClient, ok, sizeof(ok));
}

void ICACHE_FLASH_ATTR espclient_test_init(void)
{
	espclient *espClient = (espclient *)os_zalloc(sizeof(espclient));
	EspClient_Init(espClient);
	EspClient_OnConnected(espClient, espclient_test_connected_cb);
	EspClient_OnDisconnected(espClient, espclient_test_disconnected_cb);
	EspClient_OnDnsFailed(espClient, espclient_test_dnsfailed_cb);
	EspClient_OnSent(espClient, espclient_test_sent_cb);
	EspClient_OnData(espClient, espclient_test_data_cb);

	EspClient_Connect(espClient, "your-ssl-server-ip", 8443, ESPCLIENT_SEC_SSL);
}

void ICACHE_FLASH_ATTR user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000000);

	INFO("System started!\n\r");

	LED_init();
	WIFI_Connect("your-wifi-ssid", "your-wifi-password", userWifiConnectCb);
}
