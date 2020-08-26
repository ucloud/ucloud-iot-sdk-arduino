#include "UCloudIoTSDK.h"
#include <WiFi.h>
/******用户实现************/
#define wifi_ssid     ""
#define wifi_password ""

#define product_sn        ""
#define device_sn         ""
#define product_secret    ""
#define device_secret     ""
/*************************/

#define MQTT_TEST_TOPIC  "/%s/%s/test/mqtt_connect"  
#define MQTT_TEST_MESSAGE  "This is a test message!"  
char mqtt_test_topic[80];


/*******用户消息回调函数************/
void  msg_callback(char* topic, uint8_t* payload, unsigned int length)
{
  Serial.println("calling user callback ");
  Serial.print("Message received: ");
  Serial.println(topic);
  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


int wifi_connect(char *ssid,char *password)
{
  
  Serial.print("attempting to connect ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status()!= WL_CONNECTED) 
  {
   Serial.print(".");
   // wait 1 second for re-trying
   delay(1000);
  }
  Serial.print("connected with ");
  Serial.println(ssid);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

UCloudMQTT ucloudMQTT(product_sn,device_sn,product_secret,device_secret,msg_callback);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //连接wifi
  wifi_connect(wifi_ssid,wifi_password);
  //连接mqtt
  while(!ucloudMQTT.mqtt_status()) {
    ucloudMQTT.mqtt_connect();
  }
  Serial.println("connected to ucloud iot core platform!");
  //订阅topic
  sprintf(mqtt_test_topic,MQTT_TEST_TOPIC,product_sn,device_sn);
  if(ucloudMQTT.subscribe(mqtt_test_topic)==false)
    Serial.println("subscribe topic failed !");
  else
    Serial.println("subscribe topic success !");
}

void loop() {
  // put your main code here, to run repeatedly:
  ucloudMQTT.mqttYield(2000);
  if(ucloudMQTT.publish(mqtt_test_topic,MQTT_TEST_MESSAGE)==false)
    Serial.println("publish topic failed !");
  else
    Serial.println("publish topic success !");
}
