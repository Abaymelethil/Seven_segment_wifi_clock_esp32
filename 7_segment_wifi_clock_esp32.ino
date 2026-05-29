 /* MIT License
 * Copyright (c) 2026 abaymelethil

 * Hardware:  ESP32-C3 SuperMini
 * Display:   0.96" SSD1306 OLED 128x64 (I2C)
 * Pins:      SDA → GPIO 8 | SCL → GPIO 9
 * Timezone:  IST (UTC+5:30) via POSIX string
 * NTP:       in.pool.ntp.org
 *
 * https://github.com/Abaymelethil/Seven_segment_wifi_clock_esp32
 *
 * IMPORTANT — Before uploading for the first time (or if WiFi won't connect):
 *   Tools → Erase All Flash Before Sketch Upload → Enabled
 *   This clears stale NVS/WiFi credentials left in flash and prevents
 *   connectivity issues on the ESP32-C3. You can set it back to Disabled
 *   after the first successful upload.
 */

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

// --- Configuration ---
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Native POSIX timezone string for India Standard Time (IST = GMT+5:30)
const char* timezoneIST = "IST-5:30"; 
const char* ntpServer   = "in.pool.ntp.org";

// ESP32-C3 SuperMini Dedicated Pins
#define I2C_SDA 8
#define I2C_SCL 9

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Global variables for single-digit rolling tracking
int lastSecTens = 0, currentSecTens = 0;
int lastSecOnes = 0, currentSecOnes = 0;
int lastMinTens = 0, currentMinTens = 0;
int lastMinOnes = 0, currentMinOnes = 0;
int lastHourTens = 0, currentHourTens = 0;
int lastHourOnes = 0, currentHourOnes = 0;

unsigned long animStartSecTens = 0, animStartSecOnes = 0;
unsigned long animStartMinTens = 0, animStartMinOnes = 0;
unsigned long animStartHourTens = 0, animStartHourOnes = 0;

const unsigned long normalAnimDuration = 180; 
unsigned long currentAnimDuration = 350;     // Rapid boot-up roll speed

bool isAnimatingSecTens = false, isAnimatingSecOnes = false;
bool isAnimatingMinTens = false, isAnimatingMinOnes = false;
bool isAnimatingHourTens = false, isAnimatingHourOnes = false;

bool initialRollTriggered = false;

// Helper function to auto-center single-scale font strings
void drawCenteredString(String text, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, y);
  display.print(text);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;); // Halt if display allocation fails
  }
  
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  // 1. Boot Frame: Screen Brand (Zero delays)
  display.clearDisplay();
  drawCenteredString("Live WiFi Clock", 28);
  display.display();
  
  // 2. Boot Frame: Network Handshake Configuration
  display.clearDisplay();
  drawCenteredString("Connecting to WiFi...", 28);
  display.display();
  
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(100); // Fast hardware poll interval for stability
    attempts++;
  }
  
  // 3. Boot Frame: Sync internal clock environment with IST
  display.clearDisplay();
  if (WiFi.status() == WL_CONNECTED) {
    configTzTime(timezoneIST, ntpServer); // Register explicit Indian time rule
    
    struct tm timeinfo;
    int timeout = 0;
    while(!getLocalTime(&timeinfo) && timeout < 20) {
      delay(50);
      timeout++;
    }
    drawCenteredString("WiFi Connected!", 28);
  } else {
    drawCenteredString("Connection Failed", 28);
  }
  display.display();
  // All artificial delay buffers completely removed—jumps straight to clock interface
}

// Drops a single digit character smoothly UP the screen
void drawRollingSingleDigit(char current, char next, int x, int baseY, float progress) {
  int fontHeight = 16; // Height of text size 2 in pixels
  int yOffset = progress * fontHeight;
  
  // Slide old number UP and away out of sight
  display.setCursor(x, baseY - yOffset);
  display.print(current);
  
  // Pull incoming number UP into alignment from the bottom line
  display.setCursor(x, (baseY + fontHeight) - yOffset);
  display.print(next);
}

void drawClock() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int hours     = timeinfo.tm_hour;
  int minutes   = timeinfo.tm_min;
  int seconds   = timeinfo.tm_sec;
  
  // 12-Hour conversion engine
  bool isPM = (hours >= 12);
  int hours12 = hours % 12;
  if (hours12 == 0) hours12 = 12;

  // Segment individual integers for individual tracking
  int hourTens = hours12 / 10;
  int hourOnes = hours12 % 10;
  int minTens  = minutes / 10;
  int minOnes  = minutes % 10;
  int secTens  = seconds / 10;
  int secOnes  = seconds % 10;

  // --- Initial Fast Boot-Up Roll Animation ---
  if (!initialRollTriggered) {
    lastHourTens = 0; lastHourOnes = 0;
    lastMinTens  = 0; lastMinOnes  = 0;
    lastSecTens  = 0; lastSecOnes  = 0;

    currentHourTens = hourTens; currentHourOnes = hourOnes;
    currentMinTens  = minTens;  currentMinOnes  = minOnes;
    currentSecTens  = secTens;  currentSecOnes  = secOnes;

    unsigned long now = millis();
    animStartHourTens = now; animStartHourOnes = now;
    animStartMinTens  = now; animStartMinOnes  = now;
    animStartSecTens  = now; animStartSecOnes  = now;

    isAnimatingHourTens = (hourTens != 0); isAnimatingHourOnes = (hourOnes != 0);
    isAnimatingMinTens  = (minTens != 0);  isAnimatingMinOnes  = (minOnes != 0);
    isAnimatingSecTens  = (secTens != 0);  isAnimatingSecOnes  = (secOnes != 0);

    initialRollTriggered = true;
  }

  // --- Clock Matrix Pacing Controller ---
  if (initialRollTriggered && currentAnimDuration != normalAnimDuration) {
    if (!isAnimatingHourTens && !isAnimatingHourOnes && !isAnimatingMinTens && 
        !isAnimatingMinOnes && !isAnimatingSecTens && !isAnimatingSecOnes) {
      currentAnimDuration = normalAnimDuration;
    }
  }

  if (currentAnimDuration == normalAnimDuration) {
    if (secOnes != currentSecOnes) { lastSecOnes = currentSecOnes; currentSecOnes = secOnes; animStartSecOnes = millis(); isAnimatingSecOnes = true; }
    if (secTens != currentSecTens) { lastSecTens = currentSecTens; currentSecTens = secTens; animStartSecTens = millis(); isAnimatingSecTens = true; }
    if (minOnes != currentMinOnes) { lastMinOnes = currentMinOnes; currentMinOnes = minOnes; animStartMinOnes = millis(); isAnimatingMinOnes = true; }
    if (minTens != currentMinTens) { lastMinTens = currentMinTens; currentMinTens = minTens; animStartMinTens = millis(); isAnimatingMinTens = true; }
    if (hourOnes != currentHourOnes) { lastHourOnes = currentHourOnes; currentHourOnes = hourOnes; animStartHourOnes = millis(); isAnimatingHourOnes = true; }
    if (hourTens != currentHourTens) { lastHourTens = currentHourTens; currentHourTens = hourTens; animStartHourTens = millis(); isAnimatingHourTens = true; }
  }

  // Linear progression mapper
  auto getProgress = [](unsigned long start, bool &animFlag, unsigned long duration) {
    if (!animFlag) return 1.0f;
    unsigned long elapsed = millis() - start;
    if (elapsed < duration) return (float)elapsed / duration;
    animFlag = false;
    return 1.0f;
  };

  float pHT = getProgress(animStartHourTens, isAnimatingHourTens, currentAnimDuration);
  float pHO = getProgress(animStartHourOnes, isAnimatingHourOnes, currentAnimDuration);
  float pMT = getProgress(animStartMinTens, isAnimatingMinTens, currentAnimDuration);
  float pMO = getProgress(animStartMinOnes, isAnimatingMinOnes, currentAnimDuration);
  float pST = getProgress(animStartSecTens, isAnimatingSecTens, currentAnimDuration);
  float pSO = getProgress(animStartSecOnes, isAnimatingSecOnes, currentAnimDuration);

  display.clearDisplay();
  
  // --- 1. TOP BAR LAYOUT ---
  display.setFont(); 
  display.setTextSize(1);
  display.setCursor(4, 2);
  display.print("Live WiFi Clock");
  
  display.setCursor(110, 2);
  display.print(isPM ? "PM" : "AM");
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);

  // --- 2. MAIN SYMMETRICAL RUNTIME CLOCK (Size 2) ---
  display.setTextSize(2); 
  int centerY = 24; 

  // Hardcoded pixel intervals prevent digit shift when values alter widths
  int hTx = 16, hOx = 28, mTx = 52, mOx = 64, sTx = 88, sOx = 100;

  // Hours
  if (isAnimatingHourTens) drawRollingSingleDigit(String(lastHourTens)[0], String(currentHourTens)[0], hTx, centerY, pHT);
  else { display.setCursor(hTx, centerY); display.print(currentHourTens); }
  
  if (isAnimatingHourOnes) drawRollingSingleDigit(String(lastHourOnes)[0], String(currentHourOnes)[0], hOx, centerY, pHO);
  else { display.setCursor(hOx, centerY); display.print(currentHourOnes); }

  // Minutes
  if (isAnimatingMinTens) drawRollingSingleDigit(String(lastMinTens)[0], String(currentMinTens)[0], mTx, centerY, pMT);
  else { display.setCursor(mTx, centerY); display.print(currentMinTens); }
  
  if (isAnimatingMinOnes) drawRollingSingleDigit(String(lastMinOnes)[0], String(currentMinOnes)[0], mOx, centerY, pMO);
  else { display.setCursor(mOx, centerY); display.print(currentMinOnes); }

  // Seconds
  if (isAnimatingSecTens) drawRollingSingleDigit(String(lastSecTens)[0], String(currentSecTens)[0], sTx, centerY, pST);
  else { display.setCursor(sTx, centerY); display.print(currentSecTens); }

  if (isAnimatingSecOnes) drawRollingSingleDigit(String(lastSecOnes)[0], String(currentSecOnes)[0], sOx, centerY, pSO);
  else { display.setCursor(sOx, centerY); display.print(currentSecOnes); }

  // --- 3. DUAL ALIGNED RECTANGULAR COLONS ---
  if (seconds % 2 == 0 || !initialRollTriggered || currentAnimDuration != normalAnimDuration) {
    display.fillRect(44, 26, 2, 2, SSD1306_WHITE); display.fillRect(44, 34, 2, 2, SSD1306_WHITE);
    display.fillRect(80, 26, 2, 2, SSD1306_WHITE); display.fillRect(80, 34, 2, 2, SSD1306_WHITE);
  }

  // --- 4. FOOTER AUTOMATIC CENTERING ---
  display.setFont();
  display.setTextSize(1);
  String dateStr = String(dayNames[timeinfo.tm_wday]) + ", " + String(timeinfo.tm_mday) + " " + String(monthNames[timeinfo.tm_mon]) + " " + String(timeinfo.tm_year + 1900);
  drawCenteredString(dateStr, 52);

  display.display();
}

void loop() {
  drawClock();
  delay(10); // High-speed execution frequency for crisp movement tracking
}