#include "config.h"              // BLYNK_TEMPLATE_ID etc. must come first
#include <BlynkSimpleEsp32.h>
#include "blynk_handlers.h"
#include "pump_control.h"
#include "globals.h"

// Triggered automatically when the ESP32 connects to Blynk servers.
BLYNK_CONNECTED() {
  forcePumpOff();
  Blynk.virtualWrite(V12, 0);
  Blynk.syncVirtual(V12);
  Serial.println("Blynk connected. Pump forced OFF.");
}

// Triggered when V12 changes via the Blynk App switch widget.
BLYNK_WRITE(V12) {
  int v = param.asInt();
  setPump(v == 1);
  Serial.print("V12 -> Pump ");
  Serial.println(pumpOn ? "ON" : "OFF");
}
