#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

// Reads raw ADC and maps to 0-100% using SOIL_DRY/SOIL_WET calibration.
// Also caches the result into lastSoilPct.
int readSoilPercent();

#endif
