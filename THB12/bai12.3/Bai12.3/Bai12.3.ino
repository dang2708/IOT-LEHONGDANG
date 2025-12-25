/*
 * Bài thực hành 12.3: Giữ trạng thái thiết bị IoT - Retain và bài toán "nhớ/quên"
 * Code theo mẫu bài tập, đã chỉnh cho ESP32
 */

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <PubSubClient.h>

/* ===== CONFIG ===== */
const char* ssid = "DYP05";
const char* password = "12344321";

const char* mqtt_server = "192.168.1.38";  // IP Home Assistant
const int mqtt_port = 1883;

/* MQTT account (tạo trong Mosquitto HA) */
const char* mqtt_user = "yen1";
const char* mqtt_pass = "1234";

const char* student_id = "10123371";  // Mã sinh viên của bạn
/* ================== */

WiFiClient espClient;
PubSubClient client(espClient);

/* ===== TOPIC (tự động theo student_id) ===== */
String topic_cmd;
String topic_state;
String topic_note;
String topic_lwt;
/* =========================================== */

// ESP32: LED onboard là GPIO2
#if defined(ESP8266)
const int LED_PIN = LED_BUILTIN;
#else
const int LED_PIN = 2;  // GPIO2 cho ESP32
#endif

bool ledState = false;

/* ===== LED CONTROL ===== */
void applyLed(bool on) {
  ledState = on;
  
  // ESP8266: LED sáng khi LOW, tắt khi HIGH
  // ESP32: LED sáng khi HIGH, tắt khi LOW
  #if defined(ESP8266)
    digitalWrite(LED_PIN, on ? LOW : HIGH);
  #else
    digitalWrite(LED_PIN, on ? HIGH : LOW);
  #endif
  
  Serial.print("LED: ");
  Serial.println(on ? "ON" : "OFF");
}

void publishStateRetained() {
  client.publish(topic_state.c_str(), ledState ? "ON" : "OFF", true);
  Serial.print("Published state (retained): ");
  Serial.println(ledState ? "ON" : "OFF");
}

/* ===== WIFI ===== */
void setup_wifi() {
  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

/* ===== MQTT CALLBACK ===== */
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();
  
  Serial.print("MQTT [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(msg);

  if (String(topic) == topic_cmd) {
    if (msg == "ON") applyLed(true);
    else if (msg == "OFF") applyLed(false);
    else if (msg == "TOGGLE") applyLed(!ledState);
    
    publishStateRetained();
  }
}

/* ===== MQTT RECONNECT ===== */
void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP-Lab3-";
    clientId += student_id;
    
    Serial.print("Connecting MQTT... ");
    
    if (client.connect(
      clientId.c_str(),
      mqtt_user,
      mqtt_pass,
      topic_lwt.c_str(),
      0,
      true,
      "OFFLINE"
    )) {
      Serial.println("connected");
      
      client.subscribe(topic_cmd.c_str());
      Serial.print("Subscribed ");
      Serial.println(topic_cmd);
      
      // Gửi note không retain
      client.publish(topic_note.c_str(),
        "ESP BOOT / RECONNECT (note not retained)",
        false);
      Serial.println("Published non-retained note");
      
      // Gửi LWT với retain
      client.publish(topic_lwt.c_str(), "ONLINE", true);
      Serial.println("Published LWT: ONLINE (retained)");
      
      // Gửi trạng thái hiện tại với retain
      publishStateRetained();
      
    } else {
      Serial.print("failed rc=");
      Serial.println(client.state());
      delay(1500);
    }
  }
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== BÀI 12.3: MQTT RETAIN MESSAGE ===");
  Serial.print("Student ID: ");
  Serial.println(student_id);
  
  pinMode(LED_PIN, OUTPUT);
  applyLed(false);  // Tắt LED ban đầu

  /* Build topics */
  topic_cmd = "iot/lab3/" + String(student_id) + "/cmd";
  topic_state = "iot/lab3/" + String(student_id) + "/state";
  topic_note = "iot/lab3/" + String(student_id) + "/note";
  topic_lwt = "iot/lab3/" + String(student_id) + "/lwt";
  
  Serial.println("Topics:");
  Serial.print("  CMD:   ");
  Serial.println(topic_cmd);
  Serial.print("  STATE: ");
  Serial.println(topic_state);
  Serial.print("  NOTE:  ");
  Serial.println(topic_note);
  Serial.print("  LWT:   ");
  Serial.println(topic_lwt);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setKeepAlive(60);
}

/* ===== LOOP ===== */
void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}