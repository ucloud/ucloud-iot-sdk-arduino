/*
* Copyright (C) 2012-2020 UCloud. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License").
* You may not use this file except in compliance with the License.
* A copy of the License is located at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* or in the "license" file accompanying this file. This file is distributed
* on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
* express or implied. See the License for the specific language governing
* permissions and limitations under the License.
*/


#ifndef ___UCLOUDIOTSDK_H___
#define ___UCLOUDIOTSDK_H___

//#define ENABLE_ESP32_TLS
//#define ENABLE_ESP8266_TLS

#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>

#ifdef ENABLE_ESP32_TLS
#include <WiFiClientSecure.h>
#include <ssl_client.h>
#endif

#ifdef ENABLE_ESP8266_TLS
#include <WiFiClientSecure.h>
#include <time.h>
#endif





#define DYNAMIC_AUTH 1
#define STATIC_AUTH  0

#define ESP8266_TLS  2
#define ESP32_TLS    1
#define NO_TLS       0

#define FAILED_RET   1
#define SUCCESS_RET  0

typedef void (*MQTTHandlerFun)(char* topic, uint8_t* payload, unsigned int length);

typedef struct {
    char productSn[20];
	char deviceSn[20];
	char productSecret[20];
	char deviceSecret[20];
	int  auth_mode;
	int  is_tls;
	MQTTHandlerFun  user_callback;
}Uiot_Info_st;

class UCloudMQTT
{
public:
    UCloudMQTT(char    *product_sn,char *device_sn,char *product_secret,char *device_secret,MQTTHandlerFun  callback);
    ~UCloudMQTT(void);
	int       mqtt_connect(void);
	void      mqttYield(int time_ms);
	boolean   mqtt_status(void);
	boolean   publish(const char* topic, const char* payload);
	boolean   publish(const char* topic, const uint8_t * payload, unsigned int plength);
	boolean   subscribe(const char* topic);
	boolean   subscribe(const char* topic, uint8_t qos);
	boolean   unsubscribe(const char* topic);
private:
	char      clientId[40];
    char      userName[40];
	char      *passWord;
	void      gen_mqtt_connect_info(int auth_mode);
	
	#ifdef ENABLE_ESP8266_TLS 
	void      setClock();
	#endif
	
	#
};

void  dynamic_msg_callback(char* topic, uint8_t* payload, unsigned int length);

#endif /* ___UCLOUDIOTSDK_H___ */
