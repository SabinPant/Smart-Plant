#include <Arduino.h>
#include "pump_control.h"
#include "config.h"
#include "globals.h"

void setPump(bool on) {
  pumpOn = on;
  if (RELAY_LOW_TRIGGER) {
    digitalWrite(RELAY_PIN, on ? LOW : HIGH);   // LOW = closed (ON)
  } else {
    digitalWrite(RELAY_PIN, on ? HIGH : LOW);   // HIGH = closed (ON)
  }
}

void forcePumpOff() {
  if (RELAY_LOW_TRIGGER) digitalWrite(RELAY_PIN, HIGH);
  else digitalWrite(RELAY_PIN, LOW);
  pumpOn = false;
}
