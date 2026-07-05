# Smart Plant Monitoring System

An ESP32-based IoT device that monitors a plant's environment — temperature,
humidity, soil moisture, and nearby motion — and syncs everything to the
[Blynk](https://blynk.io) cloud platform. Includes a relay-controlled water
pump that can be triggered remotely from the Blynk app, plus a local I2C LCD
for at-a-glance readings.

## Hardware

| Component                                 | Purpose                       |
| ----------------------------------------- | ----------------------------- |
| ESP32-WROOM-32 DevKit                     | Main microcontroller, Wi-Fi   |
| DHT11                                     | Temperature & humidity sensor |
| Capacitive/resistive soil moisture sensor | Soil moisture (analog)        |
| PIR motion sensor                         | Detects nearby movement       |
| 16x2 I2C LCD (PCF8574 backpack)           | Local readout                 |
| 5V relay module                           | Switches the water pump       |
| Water pump + separate power source        | Waters the plant on command   |

## Wiring

| Component    | Pin  | ESP32 Pin             |
| ------------ | ---- | --------------------- |
| DHT11        | VCC  | 3V3                   |
| DHT11        | GND  | GND                   |
| DHT11        | DATA | D4                    |
| Soil sensor  | VCC  | 3V3                   |
| Soil sensor  | GND  | GND                   |
| Soil sensor  | AO   | D34                   |
| PIR sensor   | VCC  | 5V (VIN, USB-powered) |
| PIR sensor   | GND  | GND                   |
| PIR sensor   | OUT  | D27                   |
| Relay module | VCC  | 5V (VIN, USB-powered) |
| Relay module | GND  | GND                   |
| Relay module | IN   | D26                   |
| I2C LCD      | VCC  | 5V (VIN, USB-powered) |
| I2C LCD      | GND  | GND                   |
| I2C LCD      | SDA  | D21                   |
| I2C LCD      | SCL  | D22                   |

**Pump circuit (through the relay):**

```
Battery (+) → Relay NO → Relay COM → Pump wire A
Battery (–) → Pump wire B
```

The pump must be powered by its own separate supply — never straight from
the ESP32's 3V3/5V rail. The relay only switches the pump's own power line;
it does not supply power to it.

> ⚠️ **Known open issue:** the pump currently runs continuously regardless
> of relay/code state. See [Known Issues](#known-issues) below.

## Project structure

Arduino requires the `.ino` file's parent folder to share its exact name.
This sketch is split into focused modules instead of one giant file:

```
SmartPlantMonitoring/
├── SmartPlantMonitoring.ino   # setup(), loop(), sendData(), Blynk callbacks
├── config.h / config.cpp      # Blynk creds, Wi-Fi creds, pins, calibration
├── globals.h / globals.cpp    # shared runtime state (lastT, pumpOn, etc.)
├── pump_control.h / .cpp      # setPump(), forcePumpOff()
├── soil_sensor.h / .cpp       # readSoilPercent()
├── status_message.h / .cpp    # buildStatusMessage() for motion alerts
└── display_ui.h / .cpp        # LCD object + updateDisplay()
```

**Why the Blynk callbacks live in the `.ino` and not their own file:**
`BlynkSimpleEsp32.h` defines the global `Blynk` object (not just declares
it). Including that header in more than one `.cpp` file causes a
`multiple definition of 'Blynk'` linker error. So `BLYNK_CONNECTED()` and
`BLYNK_WRITE(V12)` stay in the same file that calls `Blynk.begin()`.

## Setup

1. **Install board support**
   `File > Preferences` → Additional Boards Manager URLs →
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   Then `Tools > Board > Boards Manager` → search `esp32` → install the
   Espressif package.

2. **Select the board**
   `Tools > Board > ESP32 Arduino > ESP32 Dev Module`
   (Not ESP32-C5/C3/S2/S3 — this board uses the original WROOM-32 chip.)

3. **Install libraries** (Library Manager, `Ctrl+Shift+I`)
   - `Blynk`
   - `LiquidCrystal_I2C`
   - `DHT sensor library` (plus its dependency, `Adafruit Unified Sensor`)

4. **Fill in your credentials** in `config.h` / `config.cpp`:

   ```cpp
   #define BLYNK_TEMPLATE_ID   "your-template-id"
   #define BLYNK_TEMPLATE_NAME "your-template-name"
   #define BLYNK_AUTH_TOKEN    "your-device-auth-token"
   ```

   ```cpp
   char ssid[] = "your-wifi-name";
   char pass[] = "your-wifi-password";
   ```

   Get the Blynk values from [blynk.cloud](https://blynk.cloud) under your
   Template → Device. ESP32 only supports **2.4GHz** Wi-Fi.

5. **Set up Blynk datastreams** on your template to match the virtual pins
   used in code:
   | Virtual Pin | Data |
   |---|---|
   | V0 | Temperature |
   | V1 | Humidity |
   | V3 | Soil moisture % |
   | V5 / V6 | Motion status |
   | V12 | Pump switch (read/write) |

   Also create an **Event** named exactly `motion_detected` if you want
   push notifications on motion.

6. **Calibrate the soil sensor.** Open Serial Monitor (115200 baud), note
   the `SoilRaw:` value with the probe in dry air and then in water, and
   update `SOIL_DRY` / `SOIL_WET` in `config.cpp` accordingly.

7. **Upload.** Select the correct `Tools > Port`, then hit Upload. If it
   hangs at "Connecting...", hold the board's **BOOT** button until the
   upload starts.

## Known issues

- **Pump runs continuously, ignoring relay/code state.** Confirmed via
  Serial Monitor that the ESP32 correctly believes the pump is `OFF`
  (`Pump: OFF` prints every cycle), so this is a wiring/hardware fault,
  not a code bug. Flipping `RELAY_LOW_TRIGGER` in `config.cpp` did not
  resolve it. Next steps: check for a stuck relay, confirm the IN pin
  wire is actually seated on D26, and multimeter-test the IN pin voltage
  while toggling the pump from Blynk.
- Soil moisture calibration (`SOIL_DRY` / `SOIL_WET` in `config.cpp`) uses
  placeholder values and should be recalibrated per-sensor.

## Notes

- `LiquidCrystal_I2C` will show a "claims to run on avr architecture(s)"
  warning when compiling for ESP32 — this is a harmless metadata mismatch
  in the library and does not affect functionality.
- Default I2C LCD address is `0x27`. If the screen stays blank, try `0x3F`
  (the other common default) in `display_ui.cpp`, and double-check the
  contrast potentiometer on the back of the LCD backpack.
