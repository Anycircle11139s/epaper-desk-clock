#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Wire.h>
#include "RTClib.h" // For DS3231 or internal RTC

// --- PIN CONFIGURATION (Adjust for your specific Seeed Board) ---
#define EPD_CS    D1 
#define EPD_DC    D3
#define EPD_RST   D2
#define EPD_BUSY  D0

// Change this to match your specific 2.9" panel driver (e.g., GDEW029T5)
GxEPD2_BW<GxEPD2_290_T5, GxEPD2_290_T5::HEIGHT> display(GxEPD2_290_T5(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

RTC_DS3231 rtc;
int refreshCounter = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize Display
  display.init(115200);
  display.setRotation(1); // Landscape mode
  
  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  updateDisplay();

  // Sleep logic: 60 seconds minus the time it took to refresh
  uint64_t sleepTime = 60 * 1000000; 
  display.hibernate(); 
  
  // Enter Deep Sleep (Example for ESP32-based Seeed boards)
  #ifdef ESP32
    esp_sleep_enable_timer_wakeup(sleepTime);
    esp_deep_sleep_start();
  #endif
}

void loop() {
  // Empty: Code execution ends at Deep Sleep in setup()
}

void updateDisplay() {
  DateTime now = rtc.now();
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", now.hour(), now.minute());

  display.setFont(&FreeSansBold24pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  // Center alignment math
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(timeStr, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = (display.width() - tbw) / 2;
  uint16_t y = (display.height() + tbh) / 2;

  // Logic: Full refresh every 30 mins to clean ghosting, otherwise Partial
  bool fullRefresh = (now.minute() % 30 == 0);

  if (fullRefresh) {
    display.setFullWindow();
  } else {
    display.setPartialWindow(0, 0, display.width(), display.height());
  }

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(timeStr);
    
    // Optional: Draw a thin battery line at the bottom
    display.fillRect(0, display.height() - 4, display.width(), 4, GxEPD_BLACK);
  } while (display.nextPage());
}
