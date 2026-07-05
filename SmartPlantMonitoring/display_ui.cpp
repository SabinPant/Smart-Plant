#include "display_ui.h"
#include "globals.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

void initDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void showConnectingScreen() {
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  lcd.setCursor(0, 1);
  lcd.print("WiFi+Blynk");
}

void showReadyScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
}

void updateDisplay(int soilPct, int pir) {
  // Row 0: Temperature & Humidity
  lcd.setCursor(0, 0);
  lcd.print("T:");
  if (isnan(lastT)) lcd.print("--");
  else lcd.print(lastT, 1);
  lcd.print(" H:");
  if (isnan(lastH)) lcd.print("--");
  else lcd.print(lastH, 0);
  lcd.print("  ");

  // Row 1: Soil moisture, motion, pump status
  lcd.setCursor(0, 1);
  lcd.print("Soil:");
  lcd.print(soilPct);
  lcd.print("% ");
  lcd.print("P:");
  lcd.print(pir ? "1" : "0");
  lcd.print(pumpOn ? " ON" : " OF");
}
