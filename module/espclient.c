/*
 * espclient.c
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
#include "espclient.h"

/* ------------------------------------------------------------------------------------
 * PRIVATE
 * ------------------------------------------------------------------------------------ */
// This is for SDK SSL lib. reference!
#ifdef CLIENT_SSL_ENABLE
unsigned char *default_certificate;
unsigned int default_certificate_len = 0;
unsigned char *default_private_key;
unsigned int default_private_key_len = 0;
#endif

void ICACHE_FLASH_ATTR espclient_discon_cb(void *arg);
void ICACHE_FLASH_ATTR espclient_recv_cb(void *arg, char *pdata, unsigned short len);
void ICACHE_FLASH_ATTR espclient_sent_cb(void *arg);

void ICACHE_FLASH_ATTR espclient_connect_cb(void *arg)
{
	struct espconn *conn = (struct espconn *)arg;
	espclient *espClient = (espclient *)conn->reverse;

	espconn_regist_disconcb(espClient->conn, espclient_discon_cb);
	espconn_regist_recvcb(espClient->conn, espclient_recv_cb);
	espconn_regist_sentcb(espClient->conn, espclient_sent_cb);

	if (espClient->connectedCb){
		espClient->connectedCb((uint32_t *)espClient);
	}
}

void ICACHE_FLASH_ATTR espclient_recon_cb(void *arg, sint8 err)
{
	// TODO: add attemp cnt
}

void ICACHE_FLASH_ATTR espclient_discon_cb(void *arg)
{
	struct espconn *conn = (struct espconn *)arg;
	espclient *espClient = (espclient *)conn->reverse;
	if (espClient->disconnectedCb){
		espClient->disconnectedCb((uint32_t *)espClient);
	}
}

void ICACHE_FLASH_ATTR espclient_recv_cb(void *arg, char *pdata, unsigned short len)
{
	struct espconn *conn = (struct espconn *)arg;
	espclient *espClient = (espclient *)conn->reverse;
	if (espClient->dataCb){
		espClient->dataCb((uint32_t *)espClient, pdata, len);
	}
}

void ICACHE_FLASH_ATTR espclient_sent_cb(void *arg)
{
	struct espconn *conn = (struct espconn *)arg;
	espclient *espClient = (espclient *)conn->reverse;
	if (espClient->sentCb){
		espClient->sentCb((uint32_t *)espClient);
	}
}

LOCAL void ICACHE_FLASH_ATTR espclient_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	struct espconn *conn = (struct espconn *)arg;
	espclient *espClient = (espclient *)conn->reverse;

	if(ipaddr == NULL)
	{
		// DNS: Found, but got no ip, try to reconnect\r\n");
		if (espClient->dnsFailedCb) espClient->dnsFailedCb((uint32_t *)espClient);
		return;
	}

	//	INFO("DNS: found ip %d.%d.%d.%d\n",
	//			*((uint8 *) &ipaddr->addr),
	//			*((uint8 *) &ipaddr->addr + 1),
	//			*((uint8 *) &ipaddr->addr + 2),
	//			*((uint8 *) &ipaddr->addr + 3));

	if(espClient->ip.addr == 0 && ipaddr->addr != 0)
	{
		os_memcpy(espClient->conn->proto.tcp->remote_ip, &ipaddr->addr, 4);
		if(espClient->security == ESPCLIENT_SEC_SSL){
			espconn_secure_connect(espClient->conn);
		}
		else {
			espconn_connect(espClient->conn);
		}
	}
}

/* ------------------------------------------------------------------------------------
 * PUBLIC
 * ------------------------------------------------------------------------------------ */
void ICACHE_FLASH_ATTR EspClient_Init(espclient *espClient)
{
	os_memset(espClient, 0, sizeof(espclient));
}

void ICACHE_FLASH_ATTR EspClient_Connect(espclient *espClient, char *host, uint32_t port, espclient_security_t security)
{
	uint16_t temp;
	ip_addr_t ip;

	EspClient_Disconnect(espClient);
	temp = os_strlen(host);
	espClient->host = (uint8_t*)os_zalloc(temp + 1);
	os_strcpy(espClient->host, host);
	espClient->host[temp] = 0;
	espClient->port = port;
	espClient->security = security; // TODO:

	espClient->conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
	espClient->conn->type = ESPCONN_TCP; // TODO:
	espClient->conn->state = ESPCONN_NONE;
	espClient->conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	espClient->conn->proto.tcp->local_port = espconn_port();
	espClient->conn->proto.tcp->remote_port = espClient->port;
	espClient->conn->reverse = espClient;
	espconn_regist_connectcb(espClient->conn, espclient_connect_cb);
	espconn_regist_reconcb(espClient->conn, espclient_recon_cb);

	if((ip.addr = ipaddr_addr(espClient->host)) != IPADDR_NONE) {
		os_memcpy(espClient->conn->proto.tcp->remote_ip, &ip.addr, 4);
		if(espClient->security == ESPCLIENT_SEC_SSL){
			espconn_secure_connect(espClient->conn);
		}
		else {
			espconn_connect(espClient->conn);
		}
	}
	else {
		espconn_gethostbyname(espClient->conn, espClient->host, &espClient->ip, espclient_dns_found);
	}
}

void ICACHE_FLASH_ATTR EspClient_Disconnect(espclient *espClient)
{
	if (espClient->conn){
		if (espClient->conn->type == ESPCONN_TCP && espClient->conn->proto.tcp){
			os_free(espClient->conn->proto.tcp);
		}
		// TODO:
		else if (espClient->conn->type == ESPCONN_UDP && espClient->conn->proto.udp){
			os_free(espClient->conn->proto.udp);
		}
		os_free(espClient->conn);
		espClient->conn = NULL;
	}
	if (espClient->host){
		os_free(espClient->host);
		espClient->host = NULL;
	}
}

void ICACHE_FLASH_ATTR EspClient_Send(espclient *espClient, uint8_t *pdata, uint16_t len)
{
	if(espClient->security ==  ESPCLIENT_SEC_SSL){
		espconn_secure_sent(espClient->conn, pdata, len);
	}
	else{
		espconn_sent(espClient->conn, pdata, len);
	}
}

void ICACHE_FLASH_ATTR EspClient_OnConnected(espclient *espClient, EspCallback connectedCb)
{
	if (connectedCb) espClient->connectedCb = connectedCb;
}

void ICACHE_FLASH_ATTR EspClient_OnDisconnected(espclient *espClient, EspCallback disconnectedCb)
{
	if (disconnectedCb) espClient->disconnectedCb = disconnectedCb;
}

void ICACHE_FLASH_ATTR EspClient_OnDnsFailed(espclient *espClient, EspCallback dnsFailedCb)
{
	if (dnsFailedCb) espClient->dnsFailedCb = dnsFailedCb;
}

void ICACHE_FLASH_ATTR EspClient_OnSent(espclient *espClient, EspCallback sentCb)
{
	if (sentCb) espClient->sentCb = sentCb;
}

void ICACHE_FLASH_ATTR EspClient_OnData(espclient *espClient, EspDataCallback dataCb)
{
	if (dataCb) espClient->dataCb = dataCb;
}
