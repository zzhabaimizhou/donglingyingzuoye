#include<WiFi.h>//引入WiFi库

void setup() {
  Serial.begin(115200);//初始化方便串口输出
  WiFi.begin("网络名称","网络密码");
  while(WiFi.status != WL_CONNECTED){
    delay(500);//判断是否连接，没有的话就等待500毫秒然后重新判断（说白了就是让你一直连接直到连接上）
  }
  Serial.print("IP地址：是啥就写啥")；
  Serial.print(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:

}
