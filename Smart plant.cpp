/**
 * @file SmartPlantMonitoring.ino
 * @brief Smart Plant Monitoring System IoT Device
 * * This code runs on an ESP32 microcontroller [cite: 138] and interfaces with a DHT11 
 * temperature/humidity sensor [cite: 151], a soil moisture sensor, a PIR 
 * motion sensor [cite: 156], an I2C LCD[cite: 51], and a 5V relay module to control 
 * a water pump[cite: 163, 170]. Data is synced to the Blynk IoT platform via Wi-Fi[cite: 140].
 */

// ==========================================
// BLYNK CONFIGURATION
// Must be at the very top before any includes
// ==========================================
#define BLYNK_TEMPLATE_ID   "TMPL6YnRXF8lA"
#define BLYNK_TEMPLATE_NAME "Smart Plant"
#define BLYNK_AUTH_TOKEN    "obHCGHln6K5j9_0wgWgH28VPpqbkRxEI"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ==========================================
// NETWORK CREDENTIALS
// ==========================================
char ssid[] = "Sabin";       // Wi-Fi SSID
char pass[] = "1234567890";  // Wi-Fi Password

// ==========================================
// HARDWARE PIN DEFINITIONS
// ==========================================
#define DHTPIN 4          // GPIO 4 connected to DHT11 data pin [cite: 151]
#define DHTTYPE DHT11     // Specifies the exact DHT sensor model

// Note: Report mentions GPIO 32, but code uses GPIO 34. 
// Ensure physical wiring matches this definition.
#define SOIL_AO_PIN 34    // GPIO 34 (Analog) connected to Soil Moisture AO pin
#define PIR_PIN 27        // GPIO 27 connected to PIR Motion Sensor OUT pin
#define RELAY_PIN 26      // GPIO 26 connected to 5V Relay IN pin

// ==========================================
// PERIPHERAL OBJECTS
// ==========================================
// Initialize the LCD display at I2C address 0x27 with 16 columns and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Initialize Blynk timer for non-blocking periodic data transmissions
BlynkTimer timer;

// ==========================================
// SYSTEM SETTINGS & CALIBRATION
// ==========================================
// Relay Configuration
// Most standard 5V relay modules trigger on an active-LOW signal[cite: 162].
const bool RELAY_LOW_TRIGGER = true;

// Blynk Event Configuration
// This exact string must match the event trigger created in the Blynk Web Console.
const char* MOTION_EVENT = "motion_detected";

// Anti-spam mechanism for motion events (cooldown in milliseconds)
const unsigned long MOTION_COOLDOWN_MS = 30000; // 30 seconds

// Soil Moisture Sensor Calibration Values
// Values vary depending on sensor type, soil composition, and voltage.
// Require manual calibration via Serial Monitor:
int SOIL_DRY = 3500;   // Raw ADC value when probe is exposed to dry air
int SOIL_WET = 1500;   // Raw ADC value when probe is submerged in water

// ==========================================
// GLOBAL STATE VARIABLES
// ==========================================
bool pumpOn = false;             // Tracks the current state of the water pump
int lastPir = 0;                 // Tracks previous PIR state to detect state changes (rising edge)
unsigned long lastEventMs = 0;   // Timestamp of the last logged motion event

// Cached sensor readings for alert formatting
float lastT = NAN;
float lastH = NAN;
int lastSoilPct = -1;

// ==========================================
// HELPER FUNCTIONS
// ==========================================

/**
 * @brief Safely toggles the water pump via the relay module.
 * @param on true to turn pump ON, false to turn pump OFF.
 */
void setPump(bool on) {
  pumpOn = on;
  if (RELAY_LOW_TRIGGER) {
    digitalWrite(RELAY_PIN, on ? LOW : HIGH);   // LOW = Circuit Closed (ON), HIGH = Circuit Open (OFF)
  } else {
    digitalWrite(RELAY_PIN, on ? HIGH : LOW);   // HIGH = Circuit Closed (ON), LOW = Circuit Open (OFF)
  }
}

/**
 * @brief Forces the pump to an OFF state. Used during initialization 
 * to ensure the pump doesn't turn on erratically upon boot.
 */
void forcePumpOff() {
  if (RELAY_LOW_TRIGGER) digitalWrite(RELAY_PIN, HIGH);
  else digitalWrite(RELAY_PIN, LOW);
  pumpOn = false;
}

/**
 * @brief Reads the raw analog soil moisture value and maps it to a percentage.
 * @return Soil moisture percentage (0% = Dry, 100% = Wet).
 */
int readSoilPercent() {
  int raw = analogRead(SOIL_AO_PIN);

  // Map the raw ADC value to a 0-100% scale using calibration constraints
  int pct = map(raw, SOIL_DRY, SOIL_WET, 0, 100);
  pct = constrain(pct, 0, 100);

  // Cache value for debug/email alerts
  lastSoilPct = pct;

  return pct;
}

/**
 * @brief Constructs a formatted string with current sensor readings.
 * Used to provide context when motion is detected[cite: 156].
 * @return String formatted alert message.
 */
String buildStatusMessage() {
  String msg = "Motion detected! ";

  // Append Temperature
  msg += "T=";
  if (isnan(lastT)) msg += "NA";
  else msg += String(lastT, 1);
  msg += "C, H=";

  // Append Humidity
  if (isnan(lastH)) msg += "NA";
  else msg += String(lastH, 0);
  msg += "%, Soil=";

  // Append Soil Moisture
  if (lastSoilPct < 0) msg += "NA";
  else msg += String(lastSoilPct);
  msg += "%";

  return msg;
}

// ==========================================
// BLYNK CALLBACKS
// ==========================================

/**
 * @brief Callback triggered automatically when ESP32 connects to Blynk servers.
 */
BLYNK_CONNECTED() {
  // Safety protocol: Force pump off upon successful connection
  forcePumpOff();

  // Sync virtual pin state and update mobile app UI to show pump is OFF
  Blynk.virtualWrite(V12, 0);
  Blynk.syncVirtual(V12);

  Serial.println("Blynk connected. Pump forced OFF.");
}

/**
 * @brief Callback triggered when Virtual Pin V12 state changes via the Blynk App.
 * Allows manual functioning of the pump[cite: 165, 166].
 */
BLYNK_WRITE(V12) {
  int v = param.asInt();   // Read state from switch widget (0 or 1)
  setPump(v == 1);
  Serial.print("V12 -> Pump ");
  Serial.println(pumpOn ? "ON" : "OFF");
}

// ==========================================
// MAIN LOGIC FUNCTIONS
// ==========================================

/**
 * @brief Main routine to read sensors, update LCD, and push data to Cloud.
 * Executed periodically by the BlynkTimer.
 */
void sendData() {
  // 1. Read Sensor Data
  lastT = dht.readTemperature();
  lastH = dht.readHumidity();
  int pir = digitalRead(PIR_PIN);
  int soilPct = readSoilPercent();

  // 2. Transmit Data to Blynk Virtual Pins
  if (!isnan(lastT)) Blynk.virtualWrite(V0, lastT);
  if (!isnan(lastH)) Blynk.virtualWrite(V1, lastH);
  Blynk.virtualWrite(V3, soilPct);  // Soil moisture gauge (0-100)
  Blynk.virtualWrite(V6, pir);      // Motion status indicator
  Blynk.virtualWrite(V5, pir);      // Redundant UI indicator if needed

  // 3. Update Local I2C LCD Display
  // Row 0: Temperature & Humidity
  lcd.setCursor(0, 0);
  lcd.print("T:");
  if (isnan(lastT)) lcd.print("--");
  else lcd.print(lastT, 1);
  lcd.print(" H:");
  if (isnan(lastH)) lcd.print("--");
  else lcd.print(lastH, 0);
  lcd.print("  "); // Clear trailing characters

  // Row 1: Soil Moisture, Motion, & Pump Status
  lcd.setCursor(0, 1);
  lcd.print("Soil:");
  lcd.print(soilPct);
  lcd.print("% ");
  lcd.print("P:");
  lcd.print(pir ? "1" : "0");
  lcd.print(pumpOn ? " ON" : " OF");

  // 4. Output to Serial Monitor for Debugging
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

  // 5. Handle Motion Detection Events (Rising Edge Logic)
  // Triggers only when motion is newly detected (0 -> 1)
  if (pir == 1 && lastPir == 0) {
    unsigned long now = millis();
    
    // Check against cooldown to prevent spamming the notification system
    if (now - lastEventMs > MOTION_COOLDOWN_MS) {
      lastEventMs = now;

      // Push event to Blynk cloud (can trigger automations/emails)
      Blynk.logEvent(MOTION_EVENT, buildStatusMessage());
      Serial.println("Event sent: motion_detected");
    } else {
      Serial.println("Motion detected (cooldown) - no event sent.");
    }
  }

  // Update previous PIR state for the next cycle
  lastPir = pir;
}

// ==========================================
// ARDUINO SETUP
// ==========================================
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(200);

  // Configure Pin Modes
  pinMode(PIR_PIN, INPUT);
  pinMode(SOIL_AO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  // Ensure pump defaults to OFF to prevent dry running
  forcePumpOff();

  // Initialize Sensors
  dht.begin();

  // Initialize LCD Screen
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  lcd.setCursor(0, 1);
  lcd.print("WiFi+Blynk");

  // Establish Wi-Fi & Blynk Connection
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Connection Successful UI Update
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  delay(800);
  lcd.clear();

  // Schedule the core update function to run every 2000ms (2 seconds)
  timer.setInterval(2000L, sendData);

  Serial.println("Setup complete.");
  Serial.println("TIP: Calibrate SOIL_DRY & SOIL_WET using Serial Monitor.");
}

// ==========================================
// ARDUINO LOOP
// ==========================================
void loop() {
  // Keeps the Blynk connection alive and handles incoming commands
  Blynk.run();
  
  // Evaluates timers and executes scheduled functions (like sendData)
  timer.run();
}