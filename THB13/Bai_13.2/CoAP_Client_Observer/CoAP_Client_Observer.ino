#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

// ===== CẤU HÌNH WIFI =====
const char* ssid = "DYP05";        // Thay SSID của bạn
const char* password = "12344321"; // Thay mật khẩu WiFi

// ===== CẤU HÌNH SERVER =====
IPAddress serverIP(192, 168, 1, 114);  // Thay IP máy tính chạy Node.js
const int serverPort = 5683;
const int clientPort = 5683;           // Port client lắng nghe

// ===== KHỞI TẠO COAP =====
WiFiUDP udp;
Coap coap(udp);

bool observerRegistered = false;

// ===== CALLBACK NHẬN NOTIFICATION TỪ SERVER =====
void callback_notify(CoapPacket &packet, IPAddress ip, int port) {
  // Chuyển payload thành chuỗi
  char payload[packet.payloadlen + 1];
  memcpy(payload, packet.payload, packet.payloadlen);
  payload[packet.payloadlen] = '\0';
  
  // In thông báo nhận được
  Serial.print("📩 NOTIFY from Server: ");
  Serial.println(payload);
  
  // Gửi ACK nếu cần (tùy chọn)
  coap.sendResponse(ip, port, packet.messageid, NULL, 0, 
                    COAP_CHANGED, COAP_NONE, packet.token, packet.tokenlen);
}

// ===== CALLBACK XỬ LÝ RESPONSE TỪ SERVER =====
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  char payload[packet.payloadlen + 1];
  memcpy(payload, packet.payload, packet.payloadlen);
  payload[packet.payloadlen] = '\0';
  
  if (!observerRegistered) {
    Serial.print("✅ Observer registered! Initial value: ");
    Serial.println(payload);
    observerRegistered = true;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n=== CoAP Client Observer (Push) ===");
  
  // ===== KẾT NỐI WIFI =====
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✅ WiFi connected!");
  Serial.print("Client IP: ");
  Serial.println(WiFi.localIP());
  
  // ===== KHỞI ĐỘNG COAP =====
  coap.start(clientPort);
  
  // Đăng ký callback nhận response từ server
  coap.response(callback_response);
  
  // Đăng ký endpoint để nhận notification (PUSH)
  coap.server(callback_notify, "sensor/temp/notify");
  
  delay(1000);
  
  // ===== ĐĂNG KÝ OBSERVER VỚI SERVER =====
  Serial.println("\n📡 Registering Observer with Server...");
  Serial.print("Server: ");
  Serial.print(serverIP);
  Serial.print(":");
  Serial.println(serverPort);
  
  // Gửi PUT đến endpoint observe để đăng ký (thư viện không có post)
  int msgid = coap.put(serverIP, serverPort, "sensor/temp/observe", "register");
  
  Serial.print("Message ID: ");
  Serial.println(msgid);
  Serial.println("\n⏳ Waiting for notifications from server...\n");
}

void loop() {
  // Lắng nghe gói tin CoAP
  coap.loop();
  
  // Có thể thêm code để hủy observe khi cần:
  // coap.put(serverIP, serverPort, "sensor/temp/unobserve", "unregister");
}