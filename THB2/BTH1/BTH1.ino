/* b1.1
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600); 
}

void loop() {
  // put your main code here, to run repeatedly:
Serial.println("Hello, IoT!");
  delay(1000);
}
*/
// b1.2
const int BUTTON_PIN = 2;
const int LED_PIN = 10;  
// Trạng thái hệ thống
enum SystemMode {
  CONFIG_MODE,    // Chế độ cấu hình
  RUNNING_MODE    // Chế độ vận hành
};

SystemMode currentMode = CONFIG_MODE;

// Cấu hình baudrate
const long BAUD_RATES[] = {9600, 115200};
int baudRateIndex = 0;  // 0: 9600, 1: 115200
int pressCount = 0;     // Đếm số lần nhấn trong chế độ cấu hình

// Biến xử lý nút nhấn
unsigned long buttonPressTime = 0;
bool buttonPressed = false;
bool longPressHandled = false;

// Biến điều khiển LED
unsigned long lastBlinkTime = 0;
bool ledState = false;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // Khởi tạo Serial với cau hinh mặc định
  Serial.begin(BAUD_RATES[baudRateIndex]);
  
  Serial.println("=== HE THONG UART BAUD RATE ===");
  Serial.println("Che do: CAU HINH");
  Serial.println("Nhan nut de chon cau hinh:");
  Serial.println("  1 lan -> 9600 bps");
  Serial.println("  2 lan -> 115200 bps");
  Serial.println("Giu nut >3s de xac nhan va chuyen sang che do VAN HANH");
  
  // Nhấy nhanh LED khi vào cấu hình
  blinkLedFast();
}

void loop() {
  handleButton();
  updateLED();
  
  // Xử lý dữ liệu Serial trong chế độ vận hành
  if (currentMode == RUNNING_MODE && Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    Serial.print("Da nhan: ");
    Serial.println(data);
  }
}

void handleButton() {
  int buttonState = digitalRead(BUTTON_PIN);
  
  // Phát hiện nhấn nút (active LOW)
  if (buttonState == LOW && !buttonPressed) {
    buttonPressed = true;
    buttonPressTime = millis();
    longPressHandled = false;
  }
  
  // Kiểm tra nhả nút
  if (buttonState == HIGH && buttonPressed) {
    unsigned long pressDuration = millis() - buttonPressTime;
    
    // Nhấn ngắn trong chế độ cấu hình
    if (!longPressHandled && pressDuration < 3000 && currentMode == CONFIG_MODE) {
      pressCount++;
      
      if (pressCount == 1) {
        baudRateIndex = 0;  // 9600 bps
        Serial.println("\n>>> Chon: 9600 bps");
      } else if (pressCount == 2) {
        baudRateIndex = 1;  // 115200 bps
        Serial.println(">>> Chon: 115200 bps");
        pressCount = 0;  // Reset về 0 cho lần chọn tiếp theo
      }
      
      blinkLedHalfSpeed();
    }
    
    buttonPressed = false;
  }
  
  // Kiểm tra nhấn giữ > 3s
  if (buttonPressed && !longPressHandled) {
    unsigned long pressDuration = millis() - buttonPressTime;
    
    if (pressDuration >= 3000) {
      longPressHandled = true;
      
      if (currentMode == CONFIG_MODE) {
        // Chuyển sang chế độ vận hành
        switchToRunningMode();
      } else {
        // Chuyển về chế độ cấu hình
        switchToConfigMode();
      }
    }
  }
}

void switchToRunningMode() {
  currentMode = RUNNING_MODE;
  
  // Cấu hình lại Serial với baudrate đã chọn
  Serial.end();delay(100);
  Serial.begin(BAUD_RATES[baudRateIndex]);
  delay(100);
  
  Serial.println("\n================================");
  Serial.println("CHUYEN SANG CHE DO VAN HANH");
  Serial.print("cau hinh: ");
  Serial.print(BAUD_RATES[baudRateIndex]);
  Serial.println(" bps");
  Serial.println("Giu nut >3s de tro ve che do cau hinh");
  Serial.println("================================\n");
  
  // Tắt LED trong chế độ vận hành
  digitalWrite(LED_PIN, LOW);
  
  pressCount = 0;
}

void switchToConfigMode() {
  currentMode = CONFIG_MODE;
  
  Serial.println("\n================================");
  Serial.println("TRO VE CHE DO CAU HINH");
  Serial.println("Nhan nut de chon cau hinh:");
  Serial.println("  1 lan -> 9600 bps");
  Serial.println("  2 lan -> 115200 bps");
  Serial.println("================================\n");
  
  // Nhấy nhanh LED khi vào cấu hình
  blinkLedFast();
  
  pressCount = 0;
}

void updateLED() {
  // Chỉ nhấy LED trong chế độ cấu hình
  if (currentMode == CONFIG_MODE) {
    unsigned long currentTime = millis();
    int blinkInterval;
    
    // Xác định tốc độ nháy dựa vào số lần nhấn
    if (pressCount == 0) {
      blinkInterval = 200;  // Nhấy nhanh
    } else {
      blinkInterval = 500;  // Nhấy chậm 1/2
    }
    
    if (currentTime - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = currentTime;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  }
}

void blinkLedFast() {
  lastBlinkTime = millis();
  ledState = false;
}

void blinkLedHalfSpeed() {
  lastBlinkTime = millis();
  ledState = false;
}