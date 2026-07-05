/**
 * @file SmartPlantMonitoring.ino
 * @brief Smart Plant Monitoring System IoT Device — main entry point.
 *
 * This file only orchestrates: setup(), loop(), and the periodic
 * sendData() job. All the actual logic lives in the other modules
 * (config, globals, pump_control, soil_sensor, status_message,
 * display_ui, blynk_handlers) that make up this sketch folder.
 */

#include "config.h"               // Blynk creds — must be included first
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <Wire.h>
#include <DHT.h>

#include "globals.h"
#include "pump_control.h"
#include "soil_sensor.h"
#include "status_message.h"
#include "display_ui.h"

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// ==========================================
// BLYNK CALLBACKS
// These MUST live in this file (or another file that is the ONLY
// other place BlynkSimpleEsp32.h gets included) — that header defines
// the global `Blynk` object, and including it in two different .cpp
// files causes a "multiple definition of `Blynk`" linker error.
// ==========================================

/**
 * @brief Triggered automatically when the ESP32 connects to Blynk servers.
 */
BLYNK_CONNECTED() {
  forcePumpOff();
  Blynk.virtualWrite(V12, 0);
  Blynk.syncVirtual(V12);
  Serial.println("Blynk connected. Pump forced OFF.");
}

/**
 * @brief Triggered when V12 changes via the Blynk App switch widget.
 */
BLYNK_WRITE(V12) {
  int v = param.asInt();
  setPump(v == 1);
  Serial.print("V12 -> Pump ");
  Serial.println(pumpOn ? "ON" : "OFF");
}

/**
 * @brief Reads sensors, updates LCD, pushes data to Blynk, and
 * handles motion-event logging. Runs every 2 seconds via BlynkTimer.
 */
void sendData() {
  // 1. Read sensors
  lastT = dht.readTemperature();
  lastH = dht.readHumidity();
  int pir = digitalRead(PIR_PIN);
  int soilPct = readSoilPercent();

  // 2. Push to Blynk virtual pins
  if (!isnan(lastT)) Blynk.virtualWrite(V0, lastT);
  if (!isnan(lastH)) Blynk.virtualWrite(V1, lastH);
  Blynk.virtualWrite(V3, soilPct);
  Blynk.virtualWrite(V6, pir);
  Blynk.virtualWrite(V5, pir);

  // 3. Update local LCD
  updateDisplay(soilPct, pir);

  // 4. Serial debug
  int raw = analogRead(SOIL_AO_PIN);
  Serial.print("Temp: ");
  Serial.print(isnan(lastT) ? 0 : lastT, 2);
  Serial.print(" C | Hum: ");
  Serial.print(isnan(lastH) ? 0 : lastH, 2);
  Serial.print(" % | SoilRaw: ");
  Serial.print(raw);
  Serial.print(" | Soil%: ");
  Serial.print(soilPct);
  Serial.print(" | PIR: ");
  Serial.print(pir);
  Serial.print(" | Pump: ");
  Serial.println(pumpOn ? "ON" : "OFF");

  // 5. Motion detection (rising edge) with cooldown
  if (pir == 1 && lastPir == 0) {
    unsigned long now = millis();
    if (now - lastEventMs > MOTION_COOLDOWN_MS) {
      lastEventMs = now;
      Blynk.logEvent(MOTION_EVENT, buildStatusMessage());
      Serial.println("Event sent: motion_detected");
    } else {
      Serial.println("Motion detected (cooldown) - no event sent.");
    }
  }
  lastPir = pir;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIR_PIN, INPUT);
  pinMode(SOIL_AO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  forcePumpOff();
  dht.begin();

  initDisplay();
  showConnectingScreen();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  showReadyScreen();
  delay(800);
  lcd.clear();

  timer.setInterval(2000L, sendData);

  Serial.println("Setup complete.");
  Serial.println("TIP: Calibrate SOIL_DRY & SOIL_WET using Serial Monitor.");
}

void loop() {
  Blynk.run();
  timer.run();
}
