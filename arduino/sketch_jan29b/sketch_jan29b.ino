#include <ESP8266WiFi.h>

// ⚠️ 必须改成你刚才在 cmd 里查到的电脑 IPv4 地址！
const char* host = "192.168.246.250"; 
const uint16_t port = 8080;          

WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // 你的热点信息
  WiFi.begin("xm", "Xjp20020425..");

  Serial.println("\nConnecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); 
  }
  
  // --- 关键改动：连上WiFi后立即打印自己的IP ---
  Serial.println("\nWiFi Connected!");
  Serial.print("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP()); // 打印这个地址，等下用来ping
  Serial.println("--------------------------------");
}

void loop() {
  if (!client.connected()) {
    // 打印正在尝试连接的电脑IP，方便核对
    Serial.print("Attempting TCP connection to PC: ");
    Serial.println(host); 
    
    if (client.connect(host, port)) {
      Serial.println("TCP Connected! Now transmitting data..."); 
    } else {
      Serial.println("Connection failed. Retrying in 2 seconds...");
      delay(2000); 
      return;
    }
  }

  // 透传逻辑
  while (Serial.available()) {
    client.write(Serial.read());        
  }
  while (client.available()) {
    Serial.write(client.read());        
  }
}