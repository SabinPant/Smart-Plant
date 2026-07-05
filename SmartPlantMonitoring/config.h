#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// BLYNK CONFIGURATION
// Must be defined before ANY Blynk include, in every .cpp file
// that touches the Blynk library. Include this header first, always.
// ==========================================
#define BLYNK_TEMPLATE_ID   "xxxxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "xxxxxxxxxxx"
#define BLYNK_AUTH_TOKEN    "xxxxxxxxxxx"

// ==========================================
// NETWORK CREDENTIALS
// ==========================================
extern char ssid[];
extern char pass[];

// ==========================================
// HARDWARE PIN DEFINITIONS
// ==========================================
#define DHTPIN 4          // GPIO 4 -> DHT11 data pin
#define DHTTYPE DHT11

// Note: Report mentions GPIO 32, but code uses GPIO 34.
// Ensure physical wiring matches this definition.
#define SOIL_AO_PIN 34    // GPIO 34 (Analog) -> Soil Moisture AO pin
#define PIR_PIN 27        // GPIO 27 -> PIR Motion Sensor OUT pin
#define RELAY_PIN 26      // GPIO 26 -> 5V Relay IN pin

// ==========================================
// SYSTEM SETTINGS & CALIBRATION
// ==========================================
extern const bool RELAY_LOW_TRIGGER;
extern const char* MOTION_EVENT;
extern const unsigned long MOTION_COOLDOWN_MS;

// Soil moisture calibration (recalibrate via Serial Monitor as needed)
extern int SOIL_DRY;
extern int SOIL_WET;

#endif
