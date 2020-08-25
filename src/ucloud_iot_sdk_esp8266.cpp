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

#include "ucloud_iot_sdk_esp8266.h"


#define PASSWRD_TOPIC           "/$system/%s/%s/password"
#define PASSWORD_REPLY_TOPIC    "/$system/%s/%s/password_reply"
#define CLIENT_ID               "%s.%s"
#define USER_NAME               "%s|%s|%d"

const char* mqtt_ca_crt= \
{
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDsjCCApoCCQCudOie27G3QDANBgkqhkiG9w0BAQsFADCBmjELMAkGA1UEBhMC\n"
    "Q04xETAPBgNVBAgMCFNoYW5naGFpMREwDwYDVQQHDAhTaGFuZ2hhaTEPMA0GA1UE\n"
    "CgwGVUNsb3VkMRgwFgYDVQQLDA9VQ2xvdWQgSW9ULUNvcmUxFjAUBgNVBAMMDXd3\n"
    "dy51Y2xvdWQuY24xIjAgBgkqhkiG9w0BCQEWE3Vpb3QtY29yZUB1Y2xvdWQuY24w\n"
    "HhcNMTkwNzI5MTIyMDQxWhcNMzkwNzI0MTIyMDQxWjCBmjELMAkGA1UEBhMCQ04x\n"
    "ETAPBgNVBAgMCFNoYW5naGFpMREwDwYDVQQHDAhTaGFuZ2hhaTEPMA0GA1UECgwG\n"
    "VUNsb3VkMRgwFgYDVQQLDA9VQ2xvdWQgSW9ULUNvcmUxFjAUBgNVBAMMDXd3dy51\n"
    "Y2xvdWQuY24xIjAgBgkqhkiG9w0BCQEWE3Vpb3QtY29yZUB1Y2xvdWQuY24wggEi\n"
    "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC0g9bzkQBipln/nRsUKeQcwAkU\n"
    "ZUk/VMWrg5AzBislYW9hujCGZHWXUknVHPSQM2Pk30tPeIdmupSkOJllYtFplI6U\n"
    "6kqHpVsyPn925H5zz4uP5Ty0hkkNK+rIR/YjbEjGn8loTqWk++/o6qST5QOrZLUR\n"
    "vxaWvshpce0QUcYU9xMfUPzLa6/ZfmaNHgn1aWExYMJAWDyBCyw4OxeSMUyH+ydh\n"
    "egW7VHNQuVHOYdnXiC+VYImNJ8+QAyCIZ88lP0nqVPSKTt1XXmGW6vXrWSl+/XhO\n"
    "GaHNMzlwb1kqlFx/ZagTQoQ0hpmqSUKtqPgKSqGPxY9go1Rda1m2rYc8k3gJAgMB\n"
    "AAEwDQYJKoZIhvcNAQELBQADggEBAHU0KKqEbR7uoY1tlE+MDsfx/2zXNk9gYw44\n"
    "O+xGVdbvTC298Ko4uUEwc1C+l9FaMmN/2qUPSXWsSrAYDGMS63rzaxqNuADTgmo9\n"
    "QY0ITtTf0lZTkUahVSqAxzMFaaPzSfFeP9/EaUu14T5RPQbUZMVOAEPKDNmfK4rD\n"
    "06R6dnO12be4Qlha14o67+ojaNtyZ7/ESePXA/RjO9YMkeQAoa4BdnsJCZgCFmXf\n"
    "iKvGM+50+L/qSbH5F//byLGTO1t3TWCCdBE5/Mc/QLYEXDmZM6LMHyEAw4VuinIa\n"
    "I8m1P/ceVO0RjNNBG0pDH9PH4OA7ikY81c63PBCQaYMKaiksCzs=\n"
    "-----END CERTIFICATE-----\n"
};

WiFiClient       espClient;
BearSSL::WiFiClientSecure espSecureClient;
BearSSL::X509List mqttcert(mqtt_ca_crt);

static PubSubClient client;
static const char*  mqtt_server = "mqtt-cn-sh2.iot.ucloud.cn";
char password_string[30] = "{\"RequestID\":\"1\"}";
char passwrd_topic[100];
char passwrd_reply_topic[100];
Uiot_Info_st uiot_info_st;

	
//消息回调函数，收到数据之后对数据进行处理
void  dynamic_msg_callback(char* topic, uint8_t* payload, unsigned int length)
{
  Serial.print("Message received: ");
  Serial.println(topic);

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  if((0 == strncmp(topic, passwrd_reply_topic, strlen(topic))) && ( uiot_info_st.auth_mode== DYNAMIC_AUTH))
  {
    JSONVar myObject = JSON.parse((const char *)payload);
    if (myObject.hasOwnProperty("Password"))
	{
		strncpy(uiot_info_st.deviceSecret, myObject["Password"], strlen(myObject["Password"]));
  	}
    if (JSON.typeof(myObject) == "Password")
	
    Serial.print("Password received: ");
    Serial.println(uiot_info_st.deviceSecret);
    client.disconnect();
    uiot_info_st.auth_mode = STATIC_AUTH;
  }
}

//构造函数，内部对传入的四元组进行检查以及初始化四元组,传入用户回调函数
UCloudMQTT::UCloudMQTT(char    *product_sn,char *device_sn,char *product_secret,char *device_secret,MQTTHandlerFun callback)
{
	if(product_sn==NULL||device_sn==NULL)
		Serial.println("MQTT CONSTRUCT FAILED!");
	if(device_secret!=NULL)
	{
		uiot_info_st.auth_mode=STATIC_AUTH;
		memcpy(uiot_info_st.deviceSecret,device_secret,20);
		this->passWord=uiot_info_st.deviceSecret;
	}
	else if(product_secret!=NULL)
	{
		uiot_info_st.auth_mode=DYNAMIC_AUTH;
		memcpy(uiot_info_st.productSecret,product_secret,20);
    	this->passWord=uiot_info_st.productSecret;
	}
	else
	{
		Serial.println("there is no any secret!");	
	}
	if(callback==NULL)
	{
		uiot_info_st.user_callback=dynamic_msg_callback;
	}
	else
	{
		uiot_info_st.user_callback=callback;
	}
	#ifdef ENABLE_TLS
		uiot_info_st.is_tls=IS_TLS;
	#else
		uiot_info_st.is_tls=NO_TLS;
	#endif
	memcpy(uiot_info_st.productSn,product_sn,20);
	memcpy(uiot_info_st.deviceSn,device_sn,20);
}

//mqtt连接，分为动态注册时的预认证和静态连接
int UCloudMQTT::mqtt_connect(void)
{
	int retry_time=10;
	while (!client.connected()&&retry_time) 
	{
		if(uiot_info_st.is_tls==IS_TLS)
		{
			
			espSecureClient.setTrustAnchors(&mqttcert);
			setClock();
			// if no check the CA Certification
			espSecureClient.setInsecure();
			client.setClient(espSecureClient);
			client.setServer(mqtt_server, 8883);
			Serial.println("support TLS connection!");
		}
		else
		{
			client.setClient(espClient);
			client.setServer(mqtt_server, 1883);
		}
		client.setKeepAlive(500);
	    Serial.print("MQTT connecting ...");
	    /* connect now */
	     if(uiot_info_st.auth_mode == DYNAMIC_AUTH)
	    {
		    client.setCallback(dynamic_msg_callback);
	        gen_mqtt_connect_info(DYNAMIC_AUTH);
			Serial.print("MQTT dynamic register ...");
			//如果是动态注册，等待预认证完成,auth_mode会在回调函数中改变
			while (uiot_info_st.auth_mode==DYNAMIC_AUTH&&retry_time) 
		   	{ 
		   		client.connect(this->clientId,this->userName,this->passWord);
				if(mqtt_status()==true)
				{
					Serial.println("connected");
					/* subscribe topic to get device secret */
					client.subscribe(passwrd_reply_topic);
					client.publish(passwrd_topic, password_string);
					long start_tick = millis();
					long timeout_flg=0;
					while(uiot_info_st.auth_mode == DYNAMIC_AUTH&&timeout_flg==0)
					{
						mqttYield(1000);
						if(millis()-start_tick>=10000)
							timeout_flg=1;
					}
					if(timeout_flg)
					{
						Serial.println("mqtt dynamic register failed! get device password timeout!");
						return FAILED_RET;
					}
				}
				else
				{
					Serial.print("failed, status code =");
					Serial.print(client.state());
					Serial.println("   try again in 3 seconds");
					/* Wait 3 seconds before retrying */
					delay(3000);
					retry_time--;
				}
				
			} 
			if(!retry_time)
			{
				Serial.println("mqtt dynamic register failed! retry too many times");
				return FAILED_RET;
			}
	    } 
		else 
		{
			client.setCallback(uiot_info_st.user_callback);
			gen_mqtt_connect_info(STATIC_AUTH);
			Serial.print("MQTT static connect...");
      		if (client.connect(this->clientId,this->userName,this->passWord))
			{
        		Serial.println("connected");
				return 0;
      		}
			else
			{
	        	Serial.print("failed, status code =");
	        	Serial.print(client.state());
	        	Serial.println("     try again in 3 seconds");
	        	/* Wait 3 seconds before retrying */
	        	delay(3000);
				retry_time--;
      		}
	    }
    }
	if(!retry_time)
	{
		Serial.println("mqtt connect failed! retry too many times");
		return FAILED_RET;
	}
	return SUCCESS_RET;
}

//根据四元组生成mqtt连接三要素，clientID,userName以及passward
void UCloudMQTT::gen_mqtt_connect_info(int auth_mode)
{
	sprintf(this->clientId,CLIENT_ID,uiot_info_st.productSn,uiot_info_st.deviceSn);
	if(auth_mode==DYNAMIC_AUTH)
	{
		sprintf(this->userName,USER_NAME,uiot_info_st.productSn,uiot_info_st.deviceSn,2);
		sprintf(passwrd_topic,PASSWRD_TOPIC,uiot_info_st.productSn,uiot_info_st.deviceSn);
		sprintf(passwrd_reply_topic,PASSWORD_REPLY_TOPIC,uiot_info_st.productSn,uiot_info_st.deviceSn);
	}
	else
	{
		sprintf(this->userName,USER_NAME,uiot_info_st.productSn,uiot_info_st.deviceSn,1);
		this->passWord = uiot_info_st.deviceSecret;
	}
}
// Set time via NTP, as required for x.509 validation
void UCloudMQTT::setClock()
{
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

//返回mqtt连接的状态
boolean UCloudMQTT::mqtt_status()
{
	return client.connected();
}

//mqtt延时等待消息
void UCloudMQTT::mqttYield(int time_ms)
{
	long start_tick = millis();
	while(millis()-start_tick<time_ms)
	{
		//断开则重连
		if(!client.connected())
			mqtt_connect();
		//读取消息等
		client.loop();
	}
}

//发布消息
boolean  UCloudMQTT::publish(const char* topic, const char* payload)
{
	return client.publish(topic,payload);
}

//订阅消息
boolean  UCloudMQTT::subscribe(const char* topic)
{
	return client.subscribe(topic,0);
}

//析构函数
UCloudMQTT::~UCloudMQTT(void)
{

}


