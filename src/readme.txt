Senzor sol (capacitiv)	Pin 	Alimentare  GND
Senzor Zona 1 (OUT)     A0      5 V         GND
Senzor Zona 2 (OUT)     A1      5 V         GND
Senzor Zona 3 (OUT)     A2      5 V         GND
Senzor Zona 4 (OUT)     A3      5 V         GND
Senzor Zona 5 (OUT)     A4      5 V         GND
Senzor Zona 6 (OUT)     A5      5 V         GND

## Pin Assignments and Functions

| Pin Number | Symbol/Array           | Function/Description                          |
|------------|------------------------|-----------------------------------------------|
| 2          | DHTPIN_INT             | DHT22 Interior Temperature/Humidity Sensor    |
| 3          | DHTPIN_EXT             | DHT22 Exterior Temperature/Humidity Sensor    |
| A0         | SOIL_PINS[0]           | Soil Moisture Sensor 1                        |
| A1         | SOIL_PINS[1]           | Soil Moisture Sensor 2                        |
| A2         | SOIL_PINS[2]           | Soil Moisture Sensor 3                        |
| 2          | PUMP_PINS[0], PUMP1_PIN| Water Pump 1                                  |
| 3          | PUMP_PINS[1], PUMP2_PIN| Water Pump 2                                  |
| 4          | PUMP_PINS[2], PUMP3_PIN| Water Pump 3                                  |
| 4          | VALVE_PINS[0]          | Water Valve 1                                 |
| 5          | VALVE_PINS[1]          | Water Valve 2                                 |
| 6          | VALVE_PINS[2]          | Water Valve 3                                 |
| 10         | MAIN_PUMP_PIN          | Main Water Pump                               |
| 11         | FAN_PINS[0], FAN_PIN   | Fan 1 (Primary)                               |
| 12         | FAN_PINS[1]            | Fan 2                                         |
| 7          | DOOR_PINS[0]           | Door 1 Motor Pin 1                            |
| 8          | DOOR_PINS[1]           | Door 1 Motor Pin 2                            |
| 9          | DOOR_PINS[2]           | Door 2 Motor Pin 1                            |
| 13         | DOOR_PINS[3]           | Door 2 Motor Pin 2                            |
| 22         | DOOR_LIMIT_OPEN_PINS[0]| Door 1 Limit Switch (Fully Open)              |
| 24         | DOOR_LIMIT_OPEN_PINS[1]| Door 2 Limit Switch (Fully Open)              |
| 23         | DOOR_LIMIT_CLOSED_PINS[0]| Door 1 Limit Switch (Fully Closed)           |
| 25         | DOOR_LIMIT_CLOSED_PINS[1]| Door 2 Limit Switch (Fully Closed)           |
| 12         | TRAP_UP_PIN            | Trap Motor Up                                 |
| 13         | TRAP_DOWN_PIN          | Trap Motor Down                               |
| A3         | RAIN_SENSOR_PINS[0]    | Rain Sensor                                   |

### I2C Expander (PCF8574)
| Symbol         | Function/Description                  |
|----------------|--------------------------------------|
| PCF_ADDR       | I2C Address for PCF8574 (default 0x20)|
| PCF_VALVE_START| PCF pins 0-2 for additional valves   |
| PCF_PUMP_START | PCF pins 3-5 for additional pumps    |
| PCF_FAN_START  | PCF pins 6-7 for additional fans     |

---

**Note:**  
- Some pins are shared between arrays and single defines for convenience.
- Adjust pin numbers as needed for your hardware.

