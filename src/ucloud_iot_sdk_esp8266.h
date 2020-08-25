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

#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <Arduino_JSON.h>
#include <PubSubClient.h>

//#define ENABLE_TLS

#define DYNAMIC_AUTH 1
#define STATIC_AUTH  0

#define IS_TLS       1
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
	boolean   subscribe(const char* topic);
	
private:
	char      clientId[40];
    char      userName[40];
	char      *passWord;
	void      gen_mqtt_connect_info(int auth_mode);
	void      setClock();
};

void  dynamic_msg_callback(char* topic, uint8_t* payload, unsigned int length);


#endif /* ___UCLOUDIOTSDK_H___ */
