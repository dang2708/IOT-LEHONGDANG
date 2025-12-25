#include <WiFi.h> // Thay đổi cho ESP32
#include <WiFiUdp.h>
#include <coap-simple.h>

WiFiUDP udp;
Coap coap(udp);

// Hàm callback xử lý dữ liệu khi nhận được từng Block
void callback(CoapPacket &packet, IPAddress ip, int port) {
    Serial.print("Received block, size=");
    Serial.println(packet.payloadlen);
    // Lưu ý: Trong thực tế, các block này sẽ được nối lại hoặc ghi vào Flash
}

void setup() {
    Serial.begin(115200);
    WiFi.begin("DYP05", "12344321"); // Thay SSID/PASS của bạn
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    coap.response(callback);
    coap.start();

    // Gửi yêu cầu lấy firmware từ Server
    Serial.println("Requesting firmware...");
    coap.get(
        IPAddress(192, 168, 1, 114), // Thay bằng IP máy tính của bạn
        5683,
        "firmware"
    );
}

void loop() {
    coap.loop();
}