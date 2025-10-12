---
applyTo: '**'
---

# AI Coding Agent Instructions for Greenhouse Automation Platform

## Quickstart for AI Agents

- **Big Picture:** Modular greenhouse automation. Each hardware subsystem (doors, pumps, valves, climate, irrigation, trap) is a class injected into `SolarWebServer`.
- **Central Config:** All pins/constants in `src/config.h` and `src/config.cpp`. Never hardcode pins or addresses elsewhere.
- **Security First:** All web access is protected by the `WebAuthentication` class, which handles session management and login credential validation.
- **Web Server Pattern:**
  - All HTTP requests are handled in `SolarWebServer.cpp`.
  - Session validation/authentication required for all control actions.
  - API endpoints (e.g., `/zone/1/on`, `/door/2/open`) routed in `processControlCommands()`.
  - To add a new control, update both the UI in `sendMainPage()` and the handler in `processControlCommands()`.
- **Dependency Injection:** All control classes passed by reference to `SolarWebServer`'s constructor. Never use global instances directly.
- **Build/Test:** Use PlatformIO (`platformio.ini`). Serial debug at 115200 baud. See `test/` for test patterns.


## Project Overview
This is a modular greenhouse automation system built on Arduino UNO R4 WiFi with extensive hardware integration. The system controls irrigation, climate, doors, traps, and provides web-based monitoring/control through a secure HTTP server.

**Core Technologies**: C++/Arduino, PlatformIO, I2C expansion, analog multiplexing, WiFi connectivity
**Target Hardware**: Arduino UNO R4 WiFi with dual PCF8574 I/O expanders, CD74HC4067 multiplexer, various sensors/actuators

## Architecture Principles

### 1. Centralized Configuration
- **ALL** pin assignments, constants, and global variables are defined in `src/config.h`
- Hardware pin mappings use dedicated arrays for each device type
- Global status variables are declared as `extern` in `config.h` and defined in `src/config.cpp`
- Never hardcode pins or I2C addresses in implementation files

### 2. Modular Class Design
Each major hardware subsystem has its own class with dependency injection:
- `DoorControl` - Linear actuator control with limit switches
- `PumpControl` - Water pump relay control
- `ValveControl` - Solenoid valve relay control  
- `ClimateControl` - Temperature/humidity-based fan control
- `IrrigationControl` - Automated watering based on soil moisture, rain sensor, temperature and humidity sensors
- `TrapControl` - Top cover heat control/trap motor control
- `SolarWebServer` - HTTP server with authentication and real-time status

### 3. Hardware Abstraction Layers
- **PCF8574 I2C Expanders**: Two modules (0x20, 0x21) provide 16 digital outputs for relays/motors
- **CD74HC4067 Multiplexer**: Expands analog inputs for soil/rain sensors
- **Direct Arduino Pins**: Used for DHT sensors, flow meters, limit switches, and multiplexer control

### 4. Pin Assignment Strategy
```cpp
// Example pin organization in config.h
const int PCF1_PUMP_PINS[NUM_WATER_PUMPS] = {3, 4, 5}; // PCF8574_1 pins
const int PCF2_DOOR_PINS[NUM_OF_DOORS * 2] = {0, 1, 2, 3}; // PCF8574_2 pins
const int DOOR_LIMIT_OPEN_PINS[NUM_OF_DOORS] = {4, 5}; // Direct Arduino pins
```

### 5. Networking and Web Server
- IP to be assigned dynamically via DHCP. 
- **(Future Feature)** Device discovery via mDNS for easy access. The `WiFiS3_MDNS_Server` library is a candidate for this.
  - mDNS should check if the default hostname is available, otherwise it should append a predictable unique identifier.
- **(Network Guideline)** For stable access, configure your DHCP server to reserve a static IP for the Arduino's MAC address.
  - A suggested IP range for reservations is `*.*.*.200` to `*.*.*.220`. This cannot be enforced by the device code.


## Critical Development Guidelines

### Hardware Integration Rules
1. **I2C Address Management**: PCF1 = 0x20, PCF2 = 0x21, RTC = 0x68, LCD = varies
2. **Pin Conflict Avoidance**: Never assign the same Arduino pin to multiple functions
3. **Array Bounds Safety**: Always validate array indices against `NUM_*` constants
4. **PCF8574 Pin Distribution**: 
   - PCF1 (0x20): Valves, pumps, fans (standard irrigation/climate control)
   - PCF2 (0x21): Door/trap motors and trap limit switches
5. **Multiplexer Channel Assignment**: Soil sensors (0-2), rain sensor (3), test channels (14-15)

### Code Safety Requirements
1. **Dependency Injection**: Pass PCF8574 references to control classes, never use global instances directly
2. **Status Variable Updates**: Always update corresponding global status strings when changing hardware states
3. **Error Handling**: Implement timeout mechanisms for motor operations
4. **Hardware Validation**: Check device initialization status before use

### Web Server Integration
- All control classes must be passed as references to `SolarWebServer` constructor
- Status updates must be reflected in global variables for web display
- Control commands from the web interface should delegate to appropriate control classes
- Maintain security headers and authentication for all HTTP responses
- Validate all user inputs from web requests to prevent injection attacks
- Use descriptive debug messages for hardware operations


## File Organization Standards

### Core Configuration
- `src/config.h` - Pin definitions, constants, global variable declarations
- `src/config.cpp` - Global variable definitions, simple control functions
- `src/secrets.h` - Fallback WiFi credentials. The primary method is EEPROM storage via `SecureCredentials.cpp`.

### Hardware Control Classes
- `src/DoorControl.h/.cpp` - Linear actuator control with safety features
- `src/PumpControl.h/.cpp` - Water pump relay management
- `src/ValveControl.h/.cpp` - Solenoid valve control
- `src/ClimateControl.h/.cpp` - Temperature/humidity-based automation
- `src/IrrigationControl.h/.cpp` - Soil moisture-based watering
- `src/TrapControl.h/.cpp` - Pest trap motor control

### System Services
- `src/SolarWebServer.h/.cpp` - HTTP server with authentication
- `src/WebAuthentication.h/.cpp` - Session management, login, and security logic for the web server.
- `src/HardwareInit.h/.cpp` - Hardware initialization and validation
- `src/SystemDisplay.h/.cpp` - Console and web display management
- `src/SensorData.h` - Sensor data structures
- `src/LcdDisplay.h/.cpp` - LCD display management - To be implemented

### Platform Configuration
- `platformio.ini` - Build configuration, library dependencies
- `src/readme.txt` - Hardware specifications and pin mapping documentation

## Common Development Patterns

### Adding New Hardware
1. Define pins/channels in `config.h` with appropriate constants
2. Add to hardware initialization in `HardwareInit.cpp`
3. Create control class if complex logic is needed
4. Update global status variables and web server display
5. Document in `src/readme.txt` pin mapping table

### Modifying Control Logic
1. Always work within the established control classes
2. Update status variables when hardware states change
3. Implement proper error handling and timeouts
4. Test hardware initialization sequence for any new dependencies

### Security Considerations
1. Never commit actual WiFi credentials to version control
2. Implement proper authentication for web interface
3. Validate all user inputs from web requests
4. Use security headers in HTTP responses

## Testing and Validation

### Hardware Testing Sequence
1. I2C device scan and validation
2. PCF8574 pin toggle tests
3. Multiplexer channel verification
4. Sensor reading validation
5. Motor/actuator movement tests with limit switch feedback

### Code Quality Requirements
- All public functions must have meaningful names and documentation
- Complex logic should include explanatory comments
- Error conditions must be handled gracefully
- Hardware failures should not crash the system

## Build and Deployment

### PlatformIO Configuration
- Target: `uno_r4_wifi` board with `renesas-ra` platform
- Required libraries are managed in `platformio.ini`
- Build flags include firmware version and environment definitions

### Serial Debugging
- Baud rate: 115200
- Use descriptive debug messages for hardware operations
- Include hardware status in startup sequence logs

## Security Implementation Roadmap

### Current Priority Issues
1. **mDNS Integration**: Implement mDNS for easy device discovery on the local network.
2. **Input Validation**: Continuously review and sanitize all web request parameters to prevent vulnerabilities.
3. **Hardware Safety**: Enhance emergency stops and fail-safes in control classes.

### Planned Security Features
- **(Implemented)** EEPROM credential storage via `CredentialsStorage` class.
- **(Implemented)** Session management and secure headers via `WebAuthentication` and `SolarWebServer`.
- Interactive serial setup utility for credential management
- Rate limiting for web requests
- Hardware interlock systems for safety

## Version Control and Documentation

### Commit Message Format
```
<type>(<scope>): <subject>

<body>

<footer>
```
Types: feat, fix, docs, style, refactor, test, chore
Scopes: hardware, web, security, irrigation, climate, doors, traps

### Documentation Requirements
- Update `src/readme.txt` for any pin assignment changes
- Document new hardware components with specifications
- Include wiring diagrams for complex connections
- Maintain troubleshooting guides for common issues

## Integration Testing Guidelines

### Hardware Validation Checklist
- [ ] I2C device detection and communication
- [ ] PCF8574 pin control verification
- [ ] Multiplexer channel switching accuracy
- [ ] Sensor reading consistency
- [ ] Motor operation with limit switch feedback
- [ ] Web server response and control integration
- [ ] Power consumption within acceptable limits
- [ ] System stability under continuous operation

### Code Review Focus Areas
- Hardware pin conflict detection
- Array bounds safety
- Memory usage optimization
- Error handling completeness
- Security vulnerability assessment
- Code modularity and maintainability

---

*This document should be updated as the project evolves. Always validate hardware changes against the central configuration in `config.h` and test thoroughly before deployment.*
