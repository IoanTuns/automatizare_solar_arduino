# Solar Control Platform - Arduino UNO R4 WiFi

## Used Components

- **Arduino UNO R4 WiFi**
- **DHT22 (AM2302)** – indoor/outdoor
- **Soil moisture sensors (capacitive)**
- **RTC DS3231**
- **I/O Expander PCF8574 (I²C)**
- **16-channel relay module**
- **12 V pumps × 3**
- **12 V fan**
- **Linear actuator (hatch/door)**
- **100 W solar panel**
- **Solar charge controller (MPPT 10 A)**
- **12 V 45 Ah battery**
- **DC-DC converter 12→5 V (MP1584EN)**
- **LCD 16×2 I²C (Stemma QT)**
- **Water flow sensor Yf-S201 G1/2 1-30L/min**
- **Water/air solenoid valve DC 12V**

## Pin Assignments and Functions

| Pin/Channel      | Symbol/Array                | Function/Description                          |
|------------------|----------------------------|-----------------------------------------------|
| 2                | DHTPIN_INT                 | DHT22 Indoor Temperature/Humidity Sensor      |
| 3                | DHTPIN_EXT                 | DHT22 Outdoor Temperature/Humidity Sensor     |
| MUX_CH 0         | SOIL_SENSOR1_MUX_CH        | Soil Moisture Sensor 1 (via multiplexer)      |
| MUX_CH 1         | SOIL_SENSOR2_MUX_CH        | Soil Moisture Sensor 2 (via multiplexer)      |
| MUX_CH 2         | SOIL_SENSOR3_MUX_CH        | Soil Moisture Sensor 3 (via multiplexer)      |
| MUX_CH 3         | FLOW_METER1_MUX_CH         | Water Flow Meter 1 (via multiplexer)          |
| MUX_CH 4         | FLOW_METER2_MUX_CH         | Water Flow Meter 2 (via multiplexer)          |
| MUX_CH 5         | FLOW_METER3_MUX_CH         | Water Flow Meter 3 (via multiplexer)          |
| MUX_CH 6         | RAIN_SENSOR_MUX_CH         | Rain Sensor (via multiplexer)                 |
| 23               | MUX_S0_PIN                 | Multiplexer select S0                         |
| 24               | MUX_S1_PIN                 | Multiplexer select S1                         |
| 25               | MUX_S2_PIN                 | Multiplexer select S2                         |
| 26               | MUX_S3_PIN                 | Multiplexer select S3                         |
| A0               | MUX_SIG_PIN                | Multiplexer signal output (analog read)       |
| PCF8574 Pin 0    | PCF_VALVE1_PIN             | Water Valve 1 (via I2C expander)              |
| PCF8574 Pin 1    | PCF_VALVE2_PIN             | Water Valve 2 (via I2C expander)              |
| PCF8574 Pin 2    | PCF_VALVE3_PIN             | Water Valve 3 (via I2C expander)              |
| PCF8574 Pin 3    | PCF_PUMP1_PIN              | Water Pump 1 (via I2C expander)               |
| PCF8574 Pin 4    | PCF_PUMP2_PIN              | Water Pump 2 (via I2C expander)               |
| PCF8574 Pin 5    | PCF_PUMP3_PIN              | Water Pump 3 (via I2C expander)               |
| PCF8574 Pin 6    | PCF_FAN1_PIN               | Fan 1 (via I2C expander)                      |
| PCF8574 Pin 7    | PCF_FAN2_PIN               | Fan 2 (via I2C expander)                      |
| 13, 14, 15, 16   | DOOR_PINS[]                | Door Motor Pins                               |
| 17, 18           | DOOR_LIMIT_OPEN_PINS[]     | Door Limit Switches (Fully Open)              |
| 19, 20           | DOOR_LIMIT_CLOSED_PINS[]   | Door Limit Switches (Fully Closed)            |
| 21               | TRAP_UP_PIN                | Hatch Motor Up                                |
| 22               | TRAP_DOWN_PIN              | Hatch Motor Down                              |
| A4               | PCF_SDA_PIN                | I2C Data (SDA) for PCF8574 and RTC            |
| A5               | PCF_SCL_PIN                | I2C Clock (SCL) for PCF8574 and RTC           |

### I2C Expander (PCF8574)
| Symbol         | Function/Description                  |
|----------------|--------------------------------------|
| PCF_ADDR       | I2C Address for PCF8574 (default 0x20)|
| PCF_VALVE_START| PCF pins 0-2 for additional valves   |
| PCF_PUMP_START | PCF pins 3-5 for additional pumps    |
| PCF_FAN_START  | PCF pins 6-7 for additional fans     |

## Hardware Overview

- **Arduino UNO R4 WiFi**
- **PCF8574 I2C Expander**  
  - Provides 8 digital outputs for actuators (valves, pumps, fans)
  - Connected via I2C: SDA (A4), SCL (A5)
- **CD74HC4067 16-Channel Analog/Digital Multiplexer**  
  - Expands analog inputs for sensors (soil moisture, flow meters, rain sensor)
  - Channel select pins: S0 (D23), S1 (D24), S2 (D25), S3 (D26)
  - Signal pin: SIG (A0)
- **DHT22 Sensors**  
  - Interior and exterior temperature/humidity (pins 2, 3)
- **Trap and Door Motors**  
  - Controlled via dedicated digital pins
- **Limit Switches**  
  - For door position feedback
- **RTC DS3231**  
  - Real-time clock, connected via I2C (SDA: A4, SCL: A5)

## Pin Assignments

### Multiplexer Channels
- Soil Moisture Sensors: Channels 0–2
- Flow Meters: Channels 3–5
- Rain Sensor: Channel 6

### PCF8574 Outputs
- Valve 1: Pin 0
- Valve 2: Pin 1
- Valve 3: Pin 2
- Pump 1: Pin 3
- Pump 2: Pin 4
- Pump 3: Pin 5
- Fan 1: Pin 6
- Fan 2: Pin 7

### Other Pins
- DHT22: Pins 2, 3
- Trap Motor: Pins 21 (up), 22 (down)
- Door Motors: Pins 13, 14, 15, 16
- Limit Switches: Pins 17–20

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

- All actuators (valves, pumps, fans) are controlled via the PCF8574:
  ```cpp
  pcf.digitalWrite(PCF_VALVE1_PIN, LOW); // Open Valve 1
  pcf.digitalWrite(PCF_PUMP2_PIN, HIGH); // Turn off Pump 2
  ```

## I2C Devices

- PCF8574 and RTC DS3231 share the I2C bus (A4: SDA, A5: SCL).

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