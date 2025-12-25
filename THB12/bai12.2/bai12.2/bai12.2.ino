/*
 * Bài thực hành 12.2: Điều khiển LED ESP32 qua MQTT
 * Board: ESP32 DevKit v1
 * LED onboard: GPIO2 (mặc định sáng LOW)
 */

#include <WiFi.h>
#include <PubSubClient.h>

// ========== CẤU HÌNH WIFI ==========
const char* ssid = "DYP05";      // Thay bằng tên WiFi thật
const char* password = "12344321";     // Thay bằng mật khẩu WiFi thật

// ========== CẤU HÌNH MQTT ==========
const char* mqtt_server = "192.168.1.38";  // Thay bằng IP của HA
const int mqtt_port = 1883;

// Thông tin đăng nhập MQTT (phải trùng với cấu hình trong Mosquitto)
const char* mqtt_user = "yen1";        // Username MQTT
const char* mqtt_pass = "1234";        // Password MQTT

// Mã sinh viên
const char* student_id = "10123371";            // Thay bằng mã sinh viên thật

// ========== ĐỊNH NGHĨA TOPIC ==========
// Tạo tên topic động từ student_id
String topic_cmd_str = "iot/lab2/" + String(student_id) + "/cmd";
String topic_state_str = "iot/lab2/" + String(student_id) + "/state";
const char* topic_cmd = topic_cmd_str.c_str();
const char* topic_state = topic_state_str.c_str();

// ========== KHAI BÁO CHÂN LED ==========
const int LED_PIN = 2;  // LED onboard ESP32 (GPIO2)
// Lưu ý: LED ESP32 sáng khi LOW, tắt khi HIGH

// ========== BIẾN TOÀN CỤC ==========
WiFiClient espClient;
PubSubClient client(espClient);
bool ledState = false;

// ========== HÀM KẾT NỐI WIFI ==========
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Đang kết nối WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi đã kết nối!");
    Serial.print("Địa chỉ IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nKhông thể kết nối WiFi!");
  }
}

// ========== HÀM XỬ LÝ TIN NHẮN MQTT ==========
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Tin nhắn nhận được [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Chuyển payload thành String
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Xử lý lệnh
  if (message == "ON") {
    ledState = true;
    digitalWrite(LED_PIN, LOW);  // LED ESP32 sáng với LOW
    Serial.println("LED đã BẬT");
  } 
  else if (message == "OFF") {
    ledState = false;
    digitalWrite(LED_PIN, HIGH); // LED ESP32 tắt với HIGH
    Serial.println("LED đã TẮT");
  }
  
  // Gửi lại trạng thái
  String state_msg = ledState ? "ON" : "OFF";
  client.publish(topic_state, state_msg.c_str());
  Serial.print("Đã gửi trạng thái: ");
  Serial.println(state_msg);
}

// ========== HÀM KẾT NỐI LẠI MQTT ==========
void reconnect() {
  while (!client.connected()) {
    Serial.print("Đang kết nối MQTT...");
    
    // Tạo Client ID duy nhất
    String clientId = "ESP32-Lab2-";
    clientId += student_id;
    clientId += "-";
    clientId += String(random(0xffff), HEX);
    
    // Thử kết nối
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("đã kết nối!");
      
      // Subscribe topic lệnh
      client.subscribe(topic_cmd);
      Serial.print("Đã subscribe topic: ");
      Serial.println(topic_cmd);
      
      // Gửi trạng thái ban đầu
      String state_msg = ledState ? "ON" : "OFF";
      client.publish(topic_state, state_msg.c_str());
      
    } else {
      Serial.print("thất bại, rc=");
      Serial.print(client.state());
      Serial.println(" thử lại sau 2 giây...");
      delay(2000);
    }
  }
}

// ========== SETUP ==========
void setup() {
  // Khởi tạo Serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== BÀI THỰC HÀNH 12.2 - ESP32 MQTT ===");
  Serial.print("Mã sinh viên: ");
  Serial.println(student_id);
  Serial.print("Topic cmd: ");
  Serial.println(topic_cmd);
  Serial.print("Topic state: ");
  Serial.println(topic_state);
  
  // Cấu hình LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Tắt LED ban đầu
  ledState = false;
  
  // Kết nối WiFi
  setup_wifi();
  
  // Cấu hình MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setKeepAlive(60);
  
  // Kết nối MQTT
  if (WiFi.status() == WL_CONNECTED) {
    reconnect();
  }
}

// ========== LOOP ==========
void loop() {
  // Kiểm tra WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Mất kết nối WiFi, đang thử kết nối lại...");
    setup_wifi();
    delay(1000);
  }
  
  // Kiểm tra MQTT
  if (!client.connected()) {
    reconnect();
  }
  
  // Xử lý MQTT
  client.loop();
  
  // Thêm delay nhỏ để ổn định
  delay(10);
}