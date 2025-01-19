#include <ArduinoMqttClient.h>  // 引入ArduinoMqttClient库，用于实现MQTT协议(传输/接收)的通信功能
#include <ESP8266WiFi.h>        // 引入ESP8266WiFi库，用于ESP8266模块连接WiFi网络
#include <ArduinoJson.h>       // 引入ArduinoJson库，用于处理JSON格式的数据

char ssid[] = "vivo Y53s";    // 定义WiFi网络的SSID，即网络名称
char pass[] = "987654321";    // 定义WiFi网络的密码

WiFiClient wifiClient;  // 创建一个WiFiClient对象，用于建立WiFi连接
MqttClient mqttClient(wifiClient);  // 创建一个MqttClient对象，用于建立MQTT连接，传入wifiClient对象

const char broker[]    = "a1ic4mlGik0.iot-as-mqtt.cn-shanghai.aliyuncs.com";  // 定义MQTT服务器的地址
int        port        = 1883;  // 定义MQTT服务器的端口号
const char willTopic[] = "arduino/will";
const char inTopic[]   = "/sys/a1ic4mlGik0/esp8266_dev/thing/service/property/set";  // 定义接收消息的MQTT主题
const char outTopic[]  = "/sys/a1ic4mlGik0/esp8266_dev/thing/event/property/post";  // 定义发送消息的MQTT主题

const long interval = 10000;  // 定义发送消息的时间间隔，单位为毫秒
unsigned long previousMillis = 0;  // 用于记录上次发送消息的时间

int count = 0;  // 用于计数
String inputString ="";  // 用于存储接收到的MQTT消息内容

void setup() {
  // 初始化串口通信，波特率为9600
  Serial.begin(115200);
  while (!Serial) {
    ; // 等待串口连接成功
  }
  pinMode(4,OUTPUT);  // 将ESP8266的4号引脚设置为输出模式

  WiFi.begin(ssid,pass);
  while(WiFi.status()!=WL_CONNECTED){
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("You're connected to the network");  // 连接成功后输出提示信息
  Serial.println();

  // 设置MQTT连接的客户端ID、用户名和密码
  mqttClient.setId("k29cutlynzx.esp8266_dev|securemode=2,signmethod=hmacsha256,timestamp=1737195577847|");
  mqttClient.setUsernamePassword("esp8266_dev&k29cutlynzx", "9e23d38321daf5eb08e06b37a6690c40864fa1e22a9fe457990683163080a4ac");

  // 尝试连接到MQTT服务器
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    // 如果连接失败，输出错误代码并进入死循环
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");  // 连接成功后输出提示信息
  Serial.println();

  // 设置接收MQTT消息的回调函数
  mqttClient.onMessage(onMqttMessage);

  // 订阅MQTT主题
  Serial.print("Subscribing to topic: ");
  Serial.println(inTopic);
  Serial.println();

  int subscribeQos = 1;  // 设置订阅的QoS等级为1

  mqttClient.subscribe(inTopic, subscribeQos);

  // 取消订阅可以使用mqttClient.unsubscribe(inTopic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(inTopic);
  Serial.println();
}

void loop() {
  // 轮询MQTT连接，处理MQTT消息
  mqttClient.poll();

  // 获取当前时间
  unsigned long currentMillis = millis();

  // 判断是否到达发送消息的时间间隔
  if (currentMillis - previousMillis >= interval) {
    // 更新上次发送消息的时间
    previousMillis = currentMillis;

    String payload;  // 用于存储发送的JSON消息内容
    
    payload = "{\"params\":{\"temp\":55,\"humi\":66},\"version\":\"1.0.0\"}";

    // 输出发送消息的相关信息
    Serial.print("Sending message to topic: ");
    Serial.println(outTopic);
    Serial.println(payload);

    // 设置消息的属性
    bool retained = false;
    int qos = 1;
    bool dup = false;

    // 开始发送消息
    mqttClient.beginMessage(outTopic, payload.length(), retained, qos, dup);
    mqttClient.print(payload);
    mqttClient.endMessage();

    Serial.println();

    count++;  // 计数加1
  }
}

// 定义接收MQTT消息的回调函数
void onMqttMessage(int messageSize) {
  // 输出接收到消息的相关信息
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', duplicate = ");
  Serial.print(mqttClient.messageDup() ? "true" : "false");
  Serial.print(", QoS = ");
  Serial.print(mqttClient.messageQoS());
  Serial.print(", retained = ");
  Serial.print(mqttClient.messageRetain() ? "true" : "false");
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  
  String topic = mqttClient.messageTopic();
  if(topic == "/sys/k29cutlynzx/esp8266_dev/thing/event/property/post_reply"){
    Serial.println("Ignored message from topic:/sys/k29cutlynzx/esp8266_dev/thing/event/property/post_reply");
    return;
  }

  // 读取消息内容
  while (mqttClient.available()) {
    char inChar =(char)mqttClient.read();
    inputString +=inChar;
    if(inputString.length()==messageSize) {
      // 创建JSON对象，用于解析消息内容
      DynamicJsonDocument json_msg(1024);
      DynamicJsonDocument json_item(1024);
      DynamicJsonDocument json_value(1024);

      // 反序列化JSON字符串
      deserializeJson(json_msg,inputString);

      String items = json_msg["items"];  // 获取items字段的值

      deserializeJson(json_item,items);  // 反序列化items字段的值
      String led = json_item["led"];     // 获取led字段的值

      deserializeJson(json_value,led);   // 反序列化led字段的值
      bool value = json_value["value"];  // 获取value字段的值

      // 根据value字段的值控制4号引脚的电平
      if(value ==0) {
        // 关
        Serial.println("off");
        digitalWrite(4,LOW);
      } else {
        // 开
        Serial.println("on");
        digitalWrite(4,HIGH);
      }
      inputString="";  // 清空inputString变量
    }
  }
  Serial.println();
  Serial.println();
}