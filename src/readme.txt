# Solar Control Platform - Arduino UNO R4 WiFi

## Used Components

- **Arduino UNO R4 WiFi**
- **DHT22 (AM2302) Temperature/Humidity Sensor x2**
- **CD74HC4067 16-Channel Analog/Digital Multiplexer** – indoor/outdoor
- **Soil moisture sensors (capacitive) x3**
- **RTC DS3231**
- **I/O Expander PCF8574 (I²C) x2**
- **16-channel relay module**
- **12 V pumps × 3**
- **12 V fan**
- **Linear actuator (hatch/door)**
- **100 W solar panel**
- **Solar charge controller (MPPT 10 A)**
- **12 V 45 Ah battery**
- **DC-DC converter 12→5 V (MP1584EN)**
- **LCD 16×2 I²C (Stemma QT)**
- **Water flow sensor Yf-S201 G1/2 1-30L/min x3**
- **Water/air solenoid valve DC 12V x3**

## Pin Assignments and Functions

*This table reflects the pinout defined in `config.h` and is structured to simplify hardware installation.*
| Pin/Channel      | Symbol/Array                | Function/Description                          |
|------------------|-----------------------------|-----------------------------------------------|
| **Direct Arduino Pins** | | |
| 2                | `TEMP_SENSOR_PIN_INT`       | DHT22 Indoor Temperature/Humidity Sensor      |
| 3                | `TEMP_SENSOR_PIN_EXT`       | DHT22 Outdoor Temperature/Humidity Sensor     |
| 4                | `DOOR_LIMIT_OPEN_PINS[0]`   | Door 1 Limit Switch (Fully Open)              |
| 5                | `DOOR_LIMIT_OPEN_PINS[1]`   | Door 2 Limit Switch (Fully Open)              |
| 6                | `DOOR_LIMIT_CLOSED_PINS[0]` | Door 1 Limit Switch (Fully Closed)            |
| 7                | `DOOR_LIMIT_CLOSED_PINS[1]` | Door 2 Limit Switch (Fully Closed)            |
| 8                | `MUX_S0_PIN`                | Multiplexer select S0                         |
| 9                | `MUX_S1_PIN`                | Multiplexer select S1                         |
| 10 (SPI CS)      | `SD_CS_PIN`                 | SD Card Chip Select                           |
| 12               | `WATER_FLOW_METER_PINS[0]`  | Water Flow Meter 1 (Interrupt capable)        |
| 13               | `WATER_FLOW_METER_PINS[1]`  | Water Flow Meter 2 (Interrupt capable)        |
| A1               | `WATER_FLOW_METER_PINS[2]`  | Water Flow Meter 3 (Interrupt capable)        |
| A0               | `MUX_SIG_PIN`               | Multiplexer signal output (analog read)       |
| A2               | `MUX_S2_PIN`                | Multiplexer select S2                         |
| A3               | `MUX_S3_PIN`                | Multiplexer select S3                         |
| GND              | MUX `EN` Pin                | Multiplexer Enable (must be tied to GND)      |
| A4 (SDA)         | `PCF_SDA_PIN`               | I2C Data for PCF8574s, RTC, and LCD           |
| A5 (SCL)         | `PCF_SCL_PIN`               | I2C Clock for PCF8574s, RTC, and LCD          |
| **Multiplexer (MUX) Channels** | | |
| MUX CH 0         | `SOIL_SENSOR1_MUX_CH`       | Soil Moisture Sensor 1                        |
| MUX CH 1         | `SOIL_SENSOR2_MUX_CH`       | Soil Moisture Sensor 2                        |
| MUX CH 2         | `SOIL_SENSOR3_MUX_CH`       | Soil Moisture Sensor 3                        |
| MUX CH 3         | `RAIN_SENSOR_MUX_CH`        | Rain Sensor                                   |
| MUX CH 14        | `MUX_TEST_VCC_CH`           | Multiplexer Test Channel (connect to 5V)      |
| MUX CH 15        | `MUX_TEST_GND_CH`           | Multiplexer Test Channel (connect to GND)     |
| **I2C Expander PCF1 (Address 0x20)** | | |
| PCF1 Pin 0       | `PCF1_VALVE1_PIN`           | Water Valve 1                                 |
| PCF1 Pin 1       | `PCF1_VALVE2_PIN`           | Water Valve 2                                 |
| PCF1 Pin 2       | `PCF1_VALVE3_PIN`           | Water Valve 3                                 |
| PCF1 Pin 3       | `PCF1_PUMP1_PIN`            | Water Pump 1                                  |
| PCF1 Pin 4       | `PCF1_PUMP2_PIN`            | Water Pump 2                                  |
| PCF1 Pin 5       | `PCF1_PUMP3_PIN`            | Water Pump 3                                  |
| PCF1 Pin 6       | `PCF1_FAN1_PIN`             | Fan 1                                         |
| PCF1 Pin 7       | `PCF1_FAN2_PIN`             | Fan 2                                         |
| **I2C Expander PCF2 (Address 0x21)** | | |
| PCF2 Pin 0       | `PCF2_DOOR1_EXTEND_PIN`     | Door 1 Motor (Extend)                         |
| PCF2 Pin 1       | `PCF2_DOOR1_RETRACT_PIN`    | Door 1 Motor (Retract)                        |
| PCF2 Pin 2       | `PCF2_DOOR2_EXTEND_PIN`     | Door 2 Motor (Extend)                         |
| PCF2 Pin 3       | `PCF2_DOOR2_RETRACT_PIN`    | Door 2 Motor (Retract)                        |
| PCF2 Pin 4       | `PCF2_TRAP_UP_PIN`          | Trap Motor (Up)                               |
| PCF2 Pin 5       | `PCF2_TRAP_DOWN_PIN`        | Trap Motor (Down)                             |

## Hardware Overview

- **Arduino UNO R4 WiFi**
- **Two PCF8574 I2C Expanders**  
  - PCF1 (0x20) provides 8 outputs for actuators (valves, pumps, fans).
  - PCF2 (0x21) provides outputs for door and trap motors.
  - Connected via I2C: SDA (A4), SCL (A5)
- **CD74HC4067 16-Channel Analog/Digital Multiplexer**  
  - Expands analog inputs for sensors (soil moisture, rain).
  - Channel select pins: S0 (D8), S1 (D9), S2 (A2), S3 (A3).
  - Signal pin: SIG (A0)
- **DHT22 Sensors**  
  - Interior and exterior temperature/humidity (pins 2, 3)
- **Water Flow Sensors**
  - Connected to dedicated interrupt-capable pins (12, 13, A1) for accurate pulse counting.
- **Trap and Door Motors**
  - Controlled via the second PCF8574 expander (PCF2).
- **Limit Switches**  
  - For door position feedback, connected to direct digital pins (4, 5, 6, 7).
- **RTC DS3231**  
  - Real-time clock, connected via I2C (SDA: A4, SCL: A5)

## Sensor Reading

- Analog sensors are read via the multiplexer:
  1. Select the channel using S0–S3.
  2. Read the value from SIG (A0).
- Example:
  ```cpp
  selectMuxChannel(SOIL_SENSOR1_MUX_CH);
  int soil1 = analogRead(MUX_SIG_PIN);
  ```

## Actuator Control

- Actuators are controlled via the appropriate PCF8574 expander:
  ```cpp
  pcf1.digitalWrite(PCF1_VALVE1_PIN, LOW); // Open Valve 1
  pcf1.digitalWrite(PCF1_PUMP2_PIN, HIGH); // Turn off Pump 2
  pcf2.digitalWrite(PCF2_TRAP_UP_PIN, LOW); // Move trap up
  ```

## I2C Devices

- PCF8574 and RTC DS3231 share the I2C bus (A4: SDA, A5: SCL).

## Troubleshooting

### Multiplexer (MUX) Validation Fails

If the "System Info" modal shows that the Multiplexer status is "Failed/Check Wiring", it means the startup self-test did not pass. This is almost always a hardware wiring issue.

1.  **Check the `EN` (Enable) Pin:** This is the most common cause. The `EN` pin on the CD74HC4067 board **must be connected to GND** for the chip to be active. If it is left unconnected, the MUX will be disabled.
2.  **Check Test Channel Wiring:** The self-test relies on two specific channels being connected to known voltages.
    -   Verify that **Channel 14** on the MUX is firmly connected to a **5V** pin on the Arduino.
    -   Verify that **Channel 15** on the MUX is firmly connected to a **GND** pin on the Arduino.
3.  **Check MUX Power:** Ensure the MUX board itself is receiving power. It needs its own `VCC` pin connected to **5V** and its own `GND` pin connected to **GND** on the Arduino.
4.  **Check Select Pins:** Verify that the MUX select pins (`S0`-`S3`) are correctly wired to the Arduino pins defined in `config.h` (D8, D9, A2, A3).

### SD Card Not Detected

- Verify that the SD card module is wired to the correct SPI pins (10-13).
- Ensure the card is formatted as FAT16 or FAT32.

## Notes

- Do **not** use direct Arduino pins for actuators; use PCF8574 assignments.
- Do **not** use direct analog pins for sensors; use multiplexer channels.
- All pin and channel assignments are defined in `config.h`.

---

For more details, see comments in `config.h` and the main source files.
---

**Note:**  
- Some pins are shared between arrays and single defines for convenience.
- Adjust pin numbers as needed for your hardware.