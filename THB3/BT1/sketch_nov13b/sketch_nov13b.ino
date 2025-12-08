#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// --- CẤU HÌNH --- //
const int PIN_BTN_MODE = 6;
const int PIN_BTN_UP   = 5;
const int PIN_BTN_DOWN = 4;
const int PIN_BTN_SET  = 3;
const int PIN_BUZZER   = 2;

LiquidCrystal_I2C lcd(0x27, 16, 2); 
RTC_DS1307 rtc;

// --- MODE --- //
enum ModeState {
  MODE_VIEW_TIME,
  MODE_SET_ALARM_H,
  MODE_SET_ALARM_M,
  MODE_SET_TIME_H,
  MODE_SET_TIME_M
};

ModeState currentMode = MODE_VIEW_TIME;

// --- BIẾN BÁO THỨC --- //
int alarmHour = 6;
int alarmMinute = 0;
bool isAlarmOn = true;
bool isRinging = false;

// --- BIẾN CHỈNH GIỜ tạm --- //
int tempHour, tempMinute;

// --- XỬ LÝ GIỮ NÚT MODE --- //
unsigned long modePressTime = 0;
bool modeButtonHeld = false;

// ---- PROTOTYPE ---- //
void updateTempTimeFromRTC();
void showBlinkingCursor(int index);
void handleButtons();
void changeMode();
void changeValue(int direction);
void handleSet();
void displayRealTime();
void displaySetAlarm();
void displaySetTime();
void checkAlarmTrigger();
void ringAlarm();
void stopAlarm();

// ------------------------------------------- //
//                    SETUP                    //
// ------------------------------------------- //

void setup() {
  pinMode(PIN_BTN_MODE, INPUT_PULLUP);
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
  pinMode(PIN_BTN_SET, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_BUZZER, LOW);

  lcd.init();
  lcd.backlight();

  if (!rtc.begin()) {
    lcd.print("RTC ERROR!");
    while (1);
  }

  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// ------------------------------------------- //
//                    LOOP                     //
// ------------------------------------------- //

void loop() {
  handleButtons();

  if (isRinging) {
    ringAlarm();
    return;
  }

  switch (currentMode) {
    case MODE_VIEW_TIME:
      displayRealTime();
      checkAlarmTrigger();
      break;

    case MODE_SET_ALARM_H:
    case MODE_SET_ALARM_M:
      displaySetAlarm();
      break;

    case MODE_SET_TIME_H:
    case MODE_SET_TIME_M:
      displaySetTime();
      break;
  }

  delay(50);
}

// ------------------------------------------- //
//               XỬ LÝ NÚT NHẤN               //
// ------------------------------------------- //

void handleButtons() {
  int modeState = digitalRead(PIN_BTN_MODE);

  // --- XỬ LÝ NÚT MODE --- //
  if (modeState == LOW) {
    if (modePressTime == 0) modePressTime = millis();

    // Nhấn giữ 2 giây -> bật/tắt báo thức
    if (!modeButtonHeld && (millis() - modePressTime > 2000) && currentMode == MODE_VIEW_TIME) {
      isAlarmOn = !isAlarmOn;
      modeButtonHeld = true;

      lcd.clear();
      lcd.print(isAlarmOn ? "ALARM: ON" : "ALARM: OFF");
      delay(1000);
      lcd.clear();
    }
  } 
  else {
    // Nhấn ngắn
    if (modePressTime > 0) {
      if (!modeButtonHeld && millis() - modePressTime < 1000) {
        if (isRinging)
          stopAlarm();
        else
          changeMode();
      }
      modePressTime = 0;
      modeButtonHeld = false;
    }
  }

  // --- UP/DOWN chung 1 hàm --- //
  if (digitalRead(PIN_BTN_UP) == LOW) {
    changeValue(+1);
    delay(200);
  }

  if (digitalRead(PIN_BTN_DOWN) == LOW) {
    changeValue(-1);
    delay(200);
  }

  // --- SET --- //
  if (digitalRead(PIN_BTN_SET) == LOW) {
    handleSet();
    delay(250);
  }
}

// ------------------------------------------- //
//                THAY ĐỔI MODE                //
// ------------------------------------------- //

void changeMode() {
  lcd.clear();

  switch (currentMode) {
    case MODE_VIEW_TIME:
      currentMode = MODE_SET_ALARM_H;
      tempHour = alarmHour;
      tempMinute = alarmMinute;
      break;

    case MODE_SET_ALARM_H:
    case MODE_SET_ALARM_M:
      updateTempTimeFromRTC();
      currentMode = MODE_SET_TIME_H;
      break;

    default:
      currentMode = MODE_VIEW_TIME;
      break;
  }
}

// ------------------------------------------- //
//           TĂNG / GIẢM GIÁ TRỊ GIỜ          //
// ------------------------------------------- //

void changeValue(int direction) {
  if (currentMode == MODE_SET_ALARM_H || currentMode == MODE_SET_TIME_H) {
    tempHour = (tempHour + direction + 24) % 24;
  } 
  else if (currentMode == MODE_SET_ALARM_M || currentMode == MODE_SET_TIME_M) {
    tempMinute = (tempMinute + direction + 60) % 60;
  }
}

// ------------------------------------------- //
//              XỬ LÝ NÚT SET                //
// ------------------------------------------- //

void handleSet() {
  switch (currentMode) {
    case MODE_SET_ALARM_H:
      currentMode = MODE_SET_ALARM_M;
      break;

    case MODE_SET_ALARM_M:
      alarmHour = tempHour;
      alarmMinute = tempMinute;
      isAlarmOn = true;

      lcd.clear();
      lcd.print("Saved Alarm!");
      delay(800);
      lcd.clear();

      currentMode = MODE_VIEW_TIME;
      break;

    case MODE_SET_TIME_H:
      currentMode = MODE_SET_TIME_M;
      break;

    case MODE_SET_TIME_M: {
      DateTime now = rtc.now();
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), tempHour, tempMinute, 0));
      
      lcd.clear();
      lcd.print("Time Saved!");
      delay(800);
      lcd.clear();

      currentMode = MODE_VIEW_TIME;
      break;
    }
  }
}

// ------------------------------------------- //
//                  HIỂN THỊ LCD               //
// ------------------------------------------- //

void updateTempTimeFromRTC() {
  DateTime now = rtc.now();
  tempHour = now.hour();
  tempMinute = now.minute();
}

void printDigits(int v) {
  if (v < 10) lcd.print('0');
  lcd.print(v);
}

void displayRealTime() {
  DateTime now = rtc.now();

  lcd.setCursor(0, 0);
  lcd.print("Time ");
  printDigits(now.hour()); lcd.print(":");
  printDigits(now.minute()); lcd.print(":");
  printDigits(now.second());

  lcd.setCursor(0, 1);
  lcd.print(isAlarmOn ? "ALARM ON " : "ALARM OFF");
  printDigits(alarmHour); lcd.print(":");
  printDigits(alarmMinute);
}

void displaySetAlarm() {
  lcd.setCursor(0, 0);
  lcd.print("SET ALARM");

  lcd.setCursor(0, 1);
  lcd.print(currentMode == MODE_SET_ALARM_H ? ">" : " ");
  printDigits(tempHour);
  lcd.print(":");

  lcd.print(currentMode == MODE_SET_ALARM_M ? ">" : " ");
  printDigits(tempMinute);
}

void displaySetTime() {
  lcd.setCursor(0, 0);
  lcd.print("SET TIME ");

  lcd.setCursor(0, 1);
  lcd.print(currentMode == MODE_SET_TIME_H ? ">" : " ");
  printDigits(tempHour);
  lcd.print(":");

  lcd.print(currentMode == MODE_SET_TIME_M ? ">" : " ");
  printDigits(tempMinute);
}

// ------------------------------------------- //
//                BÁO THỨC                    //
// ------------------------------------------- //

void checkAlarmTrigger() {
  if (!isAlarmOn) return;

  DateTime now = rtc.now();
  if (now.hour() == alarmHour && now.minute() == alarmMinute && now.second() == 0) {
    isRinging = true;
  }
}

void ringAlarm() {
  lcd.setCursor(0, 0);
  lcd.print("WAKE UP!!!");
  lcd.setCursor(0, 1);
  lcd.print("Press MODE");

  digitalWrite(PIN_BUZZER, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
}

void stopAlarm() {
  isRinging = false;
  digitalWrite(PIN_BUZZER, LOW);
  lcd.clear();
}
