#include "config.h"

char ssid[] = "Sabin";
char pass[] = "1234567890";

// Most standard 5V relay modules trigger on an active-LOW signal.
const bool RELAY_LOW_TRIGGER = false;

// Must match the event trigger created in the Blynk Web Console.
const char* MOTION_EVENT = "motion_detected";

// Anti-spam cooldown for motion events (ms)
const unsigned long MOTION_COOLDOWN_MS = 30000;

int SOIL_DRY = 3500;   // Raw ADC value, probe in dry air
int SOIL_WET = 1500;   // Raw ADC value, probe submerged in water
