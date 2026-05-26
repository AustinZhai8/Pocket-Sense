#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "secrets.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BME280 bme;

#define GUVA_PIN 4
#define BTN_SCROLL 16
#define BTN_SELECT 17
#define BTN_BACK 18

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const char* menuItems[] = {"Temperature", "Humidity", "Pressure", "UV", "WiFi"};
const int menuCount = 5;
int menuIndex = 0;
bool inDetail = false;

const char* tempStatus(float t) {
  if (t < 18) return "Low (Cold)";
  if (t <= 26) return "Normal";
  return "High (Hot)";
}

const char* humidStatus(float h) {
  if (h < 30) return "Low (Dry)";
  if (h <= 60) return "Normal";
  return "High (Humid)";
}

const char* pressStatus(float p) {
  if (p < 1000) return "Low";
  if (p <= 1020) return "Normal";
  return "High";
}

const char* uvStatus(float v) {
  if (v < 0.05) return "None";
  if (v < 0.20) return "Low";
  if (v < 0.40) return "Moderate";
  return "High";
}

const char* wifiStatus(int rssi) {
  if (rssi >= -50) return "Excellent";
  if (rssi >= -65) return "Good";
  if (rssi >= -75) return "Fair";
  return "Weak";
}

void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  for (int i = 0; i < menuCount; i++) {
    display.setCursor(10, i * 10);
    if (i == menuIndex) {
      display.setCursor(0, i * 10);
      display.print(">");
    }
    display.println(menuItems[i]);
  }
  display.display();
}

void drawDetail(int index) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (index == 0) {
    float t = bme.readTemperature();
    display.println("Temperature");
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(t, 1);
    display.println(" C");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.println(tempStatus(t));
  }
  else if (index == 1) {
    float h = bme.readHumidity();
    display.println("Humidity");
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(h, 1);
    display.println(" %");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.println(humidStatus(h));
  }
  else if (index == 2) {
    float p = bme.readPressure() / 100.0;
    display.println("Pressure");
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(p, 1);
    display.println(" hPa");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.println(pressStatus(p));
  }
  else if (index == 3) {
    float v = analogRead(GUVA_PIN) * (3.3 / 4095.0);
    display.println("UV Index");
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(v, 2);
    display.println(" V");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.println(uvStatus(v));
  }
  else if (index == 4) {
    display.println("WiFi");
    display.setTextSize(1);
    display.setCursor(0, 14);
    if (WiFi.status() == WL_CONNECTED) {
      int rssi = WiFi.RSSI();
      display.print("SSID: ");
      display.println(WiFi.SSID());
      display.print("RSSI: ");
      display.print(rssi);
      display.println(" dBm");
      display.print("IP: ");
      display.println(WiFi.localIP());
      display.setCursor(0, 50);
      display.println(wifiStatus(rssi));
    } else {
      display.println("Not connected");
    }
  }

  display.display();
}

void setup() {
  Serial.begin(9600);
  pinMode(BTN_SCROLL, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }

  if (!bme.begin(0x77)) {
    Serial.println("BME280 not found");
    while (true);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AmbientIQ");
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  if (WiFi.status() == WL_CONNECTED) {
    display.println("WiFi Connected!");
    display.println(WiFi.localIP());
  } else {
    display.println("WiFi Failed");
  }
  display.display();
  delay(1500);

  drawMenu();
}

void loop() {
  if (digitalRead(BTN_SCROLL) == LOW) {
    if (!inDetail) {
      menuIndex = (menuIndex + 1) % menuCount;
      drawMenu();
    }
    delay(200);
  }
  if (digitalRead(BTN_SELECT) == LOW) {
    if (!inDetail) {
      inDetail = true;
      drawDetail(menuIndex);
    }
    delay(200);
  }
  if (digitalRead(BTN_BACK) == LOW) {
    if (inDetail) {
      inDetail = false;
      drawMenu();
    } else {
      menuIndex = (menuIndex - 1 + menuCount) % menuCount;
      drawMenu();
    }
    delay(200);
  }
}