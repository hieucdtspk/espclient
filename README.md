**ESP8266 Client with SSL support**
==========
This is a simple client library wrap code for ESP8266 with SSL support!
Thanks to TuanPM for your great [esp_mqtt](https://github.com/tuanpmt/esp_mqtt) - an mqttclient lib for esp8266, it's also the good references for me to implement espclient: 

**Features:**

 * Support multiple connection (to multiple hosts).
 * Support SSL connection (max 1024 bit key size)
 * Easy to setup and use

**Compile:**
Currently, I'm using a [full devkit for esp8266 in windows](http://www.esp8266.com/viewtopic.php?f=9&t=820)
Thanks so  much, it works perfect!
This devkit already including:
- Toolchain
- SDK
- Example (with eclipse project format)
- Eclipse project files, all required environment configuration and optimized makefile!

```bash
git clone https://github.com/hieucdtspk/espclient
```

Just import existing eclipse project file after cloning source file!
And NOT forget to change your actual COM port that connected to esp8266 in makefile, or you can add 'COM' parameter in make target!

**Usage**
```c
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
```

**Create SSL Self sign**

```
openssl req -x509 -newkey rsa:1024 -keyout key.pem -out cert.pem -days XXX
```
Change XXX to number of days you want!

**SSL/TLS server for test**

```javascript
var tls = require('tls');
var fs = require('fs');

var SECURE_KEY = __dirname + '/key.pem';
var SECURE_CERT = __dirname + '/cert.pem';

var options = {
  key: fs.readFileSync(SECURE_KEY),
  cert: fs.readFileSync(SECURE_CERT),

  // This is necessary only if using the client certificate authentication.
  // requestCert: true,

  // This is necessary only if the client uses the self-signed certificate.
  //ca: [ fs.readFileSync('client-cert.pem') ]
};

var server = tls.createServer(options, function(socket) {
  console.log('server connected',
              socket.authorized ? 'authorized' : 'unauthorized');
  // socket.pipe(socket); // this is for echo back!
  // Identify this client
  socket_name = "Client " + socket.remoteAddress + ":" + socket.remotePort 
  console.log(socket_name + " connected\n");
  
  setTimeout(function(){
	// Send a nice welcome message and announce
	socket.write("Welcome " + socket_name + "\n");
  }, 1000);
  
  setInterval(function(){
	  var current = new Date();
	  socket.write("Server interval msg: tick " + current.getTime() + "\n");
  }, 5000);
  
  // Handle incoming messages from clients.
  socket.on('data', function (data) {
    console.log(socket_name + ": " + data + "\n");
  });
 
  // Remove the client from the list when it leaves
  socket.on('end', function () {
    console.log(socket_name + " disconnected!\n");
  });
  
});

server.listen(8443, function() {
  console.log('SSL server bound!\n');
});
```

**Status:** *Initial*

**Contributing:**

***Feel free to pull any request, any contribution from you is appreciated and that's my effort to improve my dream in coding!***

**Requried:**

SDK esp_iot_sdk_v0.9.4_14_12_19 or higher

**Authors:**
[HieuNT](hieucdtspk@gmail.com)


**LICENSE - "MIT License"**

The MIT License (MIT)

Copyright (c) 2015 HieuNT

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
