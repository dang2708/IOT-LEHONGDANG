#include <WiFi.h>  // Thư viện cho ESP32
#include <WiFiUdp.h>
#include <coap-simple.h>

WiFiUDP udp;
Coap coap(udp);

void callback(CoapPacket &packet, IPAddress ip, int port) {
    Serial.print("Response: ");
    Serial.println((char*)packet.payload);
}

void setup() {
    Serial.begin(115200);
    WiFi.begin("DYP05", "12344321"); // Thay đổi thông tin WiFi của bạn
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    coap.response(callback);
    coap.start();
}

void loop() {
    // Client chủ động gửi yêu cầu GET
    coap.get(
        IPAddress(192, 168, 1, 114), // Thay bằng IP máy tính chạy Node.js
        5683,
        "sensor/temp"
    );
    delay(2000); // Đợi 2 giây trước khi gửi yêu cầu tiếp theo
}