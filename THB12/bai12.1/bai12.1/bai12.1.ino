// Chọn thư viện Wifi tùy theo chip bạn dùng
#include <WiFi.h>        // Dùng cho ESP32
//#include <ESP8266WiFi.h>    // Dùng cho ESP8266

#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ====== CONFIG (THAY ĐỔI CÁC THÔNG SỐ NÀY) ====== */
const char* device_name = "ESP32"; 
const char* ssid = "DYP05";         // <--- CHANGE ME
const char* password = "12344321";        // <--- CHANGE ME

// IP của máy tính chạy Home Assistant
const char* mqtt_server = "192.168.1.38";      // <--- CHANGE ME (IP của HA)
const int mqtt_port = 1883;

// Tài khoản đã tạo ở Bước 2.2
const char* mqtt_user = "yen1";           // <--- CHANGE ME
const char* mqtt_pass = "1234";              // <--- CHANGE ME

const char* student_id = "10123371";           // <--- CHANGE ME (Mã SV)
const char* mqtt_topic = "iot/lab1/10123371/sensor"; // <--- CHANGE ME (Phải khớp với student_id)
/* ================================================ */

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.println("========== WiFi Setup ==========");
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi connection FAILED!");
  }
  Serial.println("================================");
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    String clientId = device_name;
    clientId += "_";
    clientId += student_id;

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(">>> MQTT connected successfully!");
    } else {
      Serial.print(">>> MQTT failed, rc=");
      Serial.print(client.state());
      Serial.println(" | Retrying in 2 seconds...");
      delay(2000);
    }
  }
}

void setup() {
  delay(1000);  // Wait for serial to stabilize
  
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("############################");
  Serial.println("#   ESP8266 IoT Lab 1     #");
  Serial.println("#   Student: 10123046     #");
  Serial.println("############################");
  Serial.println();
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  Serial.println("Setup complete! Starting main loop...");
  Serial.println();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Generate random sensor data
  StaticJsonDocument<200> doc;
  float temp = random(250, 350) / 10.0;
  float hum = random(400, 800) / 10.0;
  
  doc["temp"] = temp;
  doc["hum"] = hum;
  
  char payload[128];
  serializeJson(doc, payload);
  
  Serial.println("---------------------------");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");
  Serial.print("JSON: ");
  Serial.println(payload);
  
  bool published = client.publish(mqtt_topic, payload);

  Serial.print("MQTT Publish: ");
  Serial.println(published ? "SUCCESS ✓" : "FAILED ✗");
  Serial.println("---------------------------");
  Serial.println();
  
  delay(5000);
}
