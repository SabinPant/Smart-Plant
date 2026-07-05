#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <LiquidCrystal_I2C.h>

// The LCD object lives here; other files (like the .ino) can reference
// it via this extern if ever needed, but normally only updateDisplay()
// is called from outside.
extern LiquidCrystal_I2C lcd;

void initDisplay();                       // lcd.init() + backlight + boot message
void showConnectingScreen();
void showReadyScreen();
void updateDisplay(int soilPct, int pir); // main per-cycle refresh

#endif
