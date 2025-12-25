#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

/* ========== CẤU HÌNH (SỬA LẠI CHO ĐÚNG) ========== */
const char* WIFI_SSID = "DYP05";
const char* WIFI_PASS = "12344321";

// IP CỦA MÁY TÍNH CHẠY NODE.JS (Xem bằng lệnh ipconfig trên CMD)

// IPAddress coap_server_ip(10, 174, 178, 50); // <--- SỬA IP DUNG NÀY
IPAddress coap_server_ip(192, 168, 1, 97); // <--- SỬA IP TOI NÀY

const int coap_port = 5683;

WiFiUDP udp;
Coap coap(udp);

unsigned long lastSend = 0;

#ifndef COAP_CON
#define COAP_CON (COAP_TYPE)0
#endif

#ifndef COAP_NON
#define COAP_NON (COAP_TYPE)1
#endif

void setup() {
  Serial.begin(115200);
  
  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP ESP32: "); Serial.println(WiFi.localIP());

  // Khởi động CoAP
  coap.start();
}

void loop() {
  // Quan trọng: Để nhận phản hồi từ Server
  coap.loop();

  if (millis() - lastSend >= 5000) { // Gửi mỗi 5 giây
    lastSend = millis();

    // Tạo dữ liệu nhiệt độ giả
    String payload = "temp=" + String(random(250, 350) / 10.0);
    Serial.print("Dang gui: "); Serial.println(payload);

    /* ===========================================================
       CHỌN CHẾ ĐỘ GỬI (Bỏ comment dòng bạn muốn test)
       Lưu ý hàm send có 9 tham số:
       (IP, Port, URL, Type, Method, Token, TokenLen, Payload, PayloadLen)
       =========================================================== */

    // --- CÁCH 1: GỬI NON (Không cần xác nhận) ---
    
    coap.send(coap_server_ip, coap_port, "sensor/temp", 
              COAP_NON, COAP_POST, NULL, 0, 
              (uint8_t*)payload.c_str(), payload.length());

    // --- CÁCH 2: GỬI CON (Cần xác nhận - Dùng để test tin cậy) ---
    // Đang bật mặc định cái này để bạn test bài tập
    // coap.send(coap_server_ip, coap_port, "sensor/temp", 
    //           COAP_CON, COAP_POST, NULL, 0, 
    //           (uint8_t*)payload.c_str(), payload.length());
              
  }
}