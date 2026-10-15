#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <RTClib.h>
#include <DHT.h>

// 1. GUNAKAN MODE PAGE BUFFER (_1_) AGAR TIDAK KEHABISAN RAM
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// 2. KONFIGURASI RTC & SENSOR
RTC_DS1307 rtc; // Ubah ke RTC_DS3231 rtc; jika kamu memakai DS3231

const char daysOfTheWeek[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

#define DHTPIN 2        
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

#define BUZZER_PIN 3    
#define BUTTON_PIN 4    

const int alarmHour = 22;      
const int alarmMinute = 33;    

bool isAlarmRinging = false;
bool alarmDismissed = false;

void setup() {
  Wire.begin();
  Wire.setClock(100000);

  u8g2.setI2CAddress(0x3C * 2);
  u8g2.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();

  if (rtc.begin()) {
    if (!rtc.isrunning()) {
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }

  // Splash Screen Awal
  u8g2.firstPage();
  do {
    u8g2.drawRFrame(4, 4, 120, 56, 6);
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(12, 28, "DESK STATION");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(44, 46, "READY!");
  } while (u8g2.nextPage());
  
  delay(1200);
}

void loop() {
  DateTime now = rtc.now();
  int h = now.hour();
  int m = now.minute();
  int s = now.second();

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  // Logika Alarm
  if (m != alarmMinute) {
    alarmDismissed = false;
  }

  if (h == alarmHour && m == alarmMinute && !alarmDismissed) {
    isAlarmRinging = true;
  } else {
    isAlarmRinging = false;
  }

  if (digitalRead(BUTTON_PIN) == LOW) {
    if (isAlarmRinging) {
      isAlarmRinging = false;
      alarmDismissed = true;
      digitalWrite(BUZZER_PIN, LOW);
    }
    delay(50);
  }

  if (isAlarmRinging) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Formating Teks
  char timeBuf[10];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", h, m, s);

  char dateBuf[20];
  snprintf(dateBuf, sizeof(dateBuf), "%s, %02d/%02d", daysOfTheWeek[now.dayOfTheWeek()], now.day(), now.month());

  char tempBuf[10];
  if (isnan(temp)) strcpy(tempBuf, "-- C");
  else { dtostrf(temp, 4, 1, tempBuf); strcat(tempBuf, " C"); }

  char humBuf[10];
  if (isnan(hum)) strcpy(humBuf, "-- %");
  else snprintf(humBuf, sizeof(humBuf), "%d %%RH", (int)hum);

  // --------------------------------------------------
  // TAMPILAN PAGE LOOP (Paling Stabil di Arduino Nano)
  // --------------------------------------------------
  u8g2.firstPage();
  do {
    // 1. JAM UTAMA
    u8g2.setFont(u8g2_font_helvB18_tr);
    u8g2.drawStr(10, 22, timeBuf);

    // 2. TANGGAL & HARI
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(32, 33, dateBuf);

    // Garis Pembatas Horisontal
    u8g2.drawHLine(0, 36, 128);

    // 3. KARTU SUHU
    u8g2.drawRFrame(0, 39, 61, 25, 4);
    u8g2.setFont(u8g2_font_micro_tr);
    u8g2.drawStr(4, 46, "TEMP");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 59, tempBuf);

    // 4. KARTU KELEMBAPAN
    u8g2.drawRFrame(67, 39, 61, 25, 4);
    u8g2.setFont(u8g2_font_micro_tr);
    u8g2.drawStr(71, 46, "HUMID");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(71, 59, humBuf);

  } while (u8g2.nextPage());

  delay(200);
}