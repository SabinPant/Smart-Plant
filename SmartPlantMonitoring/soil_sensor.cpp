#include <Arduino.h>
#include "soil_sensor.h"
#include "config.h"
#include "globals.h"

int readSoilPercent() {
  int raw = analogRead(SOIL_AO_PIN);

  int pct = map(raw, SOIL_DRY, SOIL_WET, 0, 100);
  pct = constrain(pct, 0, 100);

  lastSoilPct = pct;
  return pct;
}
