#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

extern bool pumpOn;             // Current pump state
extern int lastPir;             // Previous PIR reading (edge detection)
extern unsigned long lastEventMs; // Timestamp of last motion event

// Cached sensor readings, used for alert formatting / LCD
extern float lastT;
extern float lastH;
extern int lastSoilPct;

#endif
