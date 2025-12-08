#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 10

MD_Parola parola = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

String msg = "HELLO";       // Chuỗi mặc định
bool hasNewMsg = false;

void setup() {
  Serial.begin(9600);

  parola.begin();
  parola.displayClear();

  // Hiển thị chuỗi mặc định ban đầu
  parola.displayText((char*)msg.c_str(), PA_CENTER, 80, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  Serial.println("Nhap chuoi roi nhan Enter:");
}

void loop() {
  // Nhận dữ liệu từ Virtual Terminal (Proteus)
  if (Serial.available() > 0) {
    msg = Serial.readStringUntil('\n');
    msg.trim();

    if (msg != "") {
      hasNewMsg = true;
    }
  }

  // Khi có chuỗi mới
  if (hasNewMsg) {
    parola.displayClear();
    parola.displayReset();

    // Hiển thị chuỗi vừa nhập
    parola.displayText((char*)msg.c_str(), PA_CENTER, 25, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

    hasNewMsg = false;
  }

  // Chạy hiệu ứng Parola
  if (parola.displayAnimate()) {
    parola.displayReset();
  }
}
