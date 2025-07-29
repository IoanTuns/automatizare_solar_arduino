#include <RTClib.h>
#include "HardwareInit.h"
#include <SD.h>

void HardwareInit::initializePins() {
  Serial.println("Setting door limit switch pins...");
  for (int i = 0; i < NUM_OF_DOORS; i++) {
    Serial.print("Door "); Serial.print(i+1); Serial.print(" open pin: "); Serial.println(DOOR_LIMIT_OPEN_PINS[i]);
    pinMode(DOOR_LIMIT_OPEN_PINS[i], INPUT_PULLUP);
    Serial.print("Door "); Serial.print(i+1); Serial.print(" closed pin: "); Serial.println(DOOR_LIMIT_CLOSED_PINS[i]);
    pinMode(DOOR_LIMIT_CLOSED_PINS[i], INPUT_PULLUP);
  }

  Serial.println("Setting multiplexer select pins...");
  pinMode(MUX_S0_PIN, OUTPUT);
  pinMode(MUX_S1_PIN, OUTPUT);
  pinMode(MUX_S2_PIN, OUTPUT);
  pinMode(MUX_S3_PIN, OUTPUT); // Only necessary if using a 16-channel MUX or for future expansion

  Serial.println("Setting water flow sensor pins and attaching interrupts...");
  // Attach interrupts for each flow meter pin.
  // Ensure the ISRs (flowMeterISR0, flowMeterISR1, flowMeterISR2) are defined globally.
  // RISING is generally suitable for pulse sensors, but check your sensor's datasheet.
  pinMode(WATER_FLOW_METER_PINS[0], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_METER_PINS[0]), flowMeterISR0, RISING);
  Serial.print("Flow Meter 1 on pin "); Serial.println(WATER_FLOW_METER_PINS[0]);

  pinMode(WATER_FLOW_METER_PINS[1], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_METER_PINS[1]), flowMeterISR1, RISING);
  Serial.print("Flow Meter 2 on pin "); Serial.println(WATER_FLOW_METER_PINS[1]);

  pinMode(WATER_FLOW_METER_PINS[2], INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_METER_PINS[2]), flowMeterISR2, RISING);
  Serial.print("Flow Meter 3 on pin "); Serial.println(WATER_FLOW_METER_PINS[2]);

  Serial.println("Pins initialized successfully");
}

// Initialize RTC with retry
DateTime HardwareInit::initializeRTC(RTC_DS3231& rtc, bool setTime) {
  Serial.print("Initializing RTC...");

  // Pre-check to see if a device is actually present at the RTC clock's I2C address
  Wire.beginTransmission(RTC_CLOCK_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("\nERROR: No device responded at the RTC clock address (0x" + String(RTC_CLOCK_ADDR, HEX) + ").");
    Serial.println("Please check RTC module wiring and power.");
    return DateTime((uint32_t)0); // Return invalid time immediately
  }

  int attempts = 0;
  const int maxAttempts = 5;
  while (attempts < maxAttempts) {
    if (rtc.begin()) {
      if (rtc.lostPower() || setTime) {
        Serial.println("RTC lost power or setTime requested, setting time...");
        rtc.adjust(DateTime(__DATE__, __TIME__));
      }
      Serial.println("RTC OK");
      return rtc.now();
    }
    attempts++;
    Serial.print(".");
    delay(1000);
  }
  Serial.println("RTC FAILED!");
  Serial.println("WARNING: RTC not found after 5 attempts. Continuing without RTC...");
  return DateTime((uint32_t)0);
}

bool HardwareInit::initializeSD(int chipSelect) {
  Serial.print("Initializing SD card...");
  bool sdInitialized = SD.begin(chipSelect);
  if (!sdInitialized) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card initialized.");
  }
  return sdInitialized;
}

bool HardwareInit::validateMux() {
    Serial.println("Validating Multiplexer (CD74HC4067)...");

    // First, let's check what we read when no specific channel is selected
    Serial.println("=== MUX Diagnostic Information ===");
    Serial.println("MUX Pin Configuration:");
    Serial.println("  S0 (Pin " + String(MUX_S0_PIN) + ")");
    Serial.println("  S1 (Pin " + String(MUX_S1_PIN) + ")");
    Serial.println("  S2 (Pin " + String(MUX_S2_PIN) + ")");
    Serial.println("  S3 (Pin " + String(MUX_S3_PIN) + ")");
    Serial.println("  SIG (Pin A" + String(MUX_SIG_PIN - A0) + ")");
    Serial.println("  Test Channels: GND=" + String(MUX_TEST_GND_CH) + ", VCC=" + String(MUX_TEST_VCC_CH));

    // Before doing channel scan, test if basic analog read works
    Serial.println("=== Basic Analog Test ===");
    Serial.println("Testing direct analog read on pin A" + String(MUX_SIG_PIN - A0) + " (should be floating ~512):");
    
    // Set all MUX select pins to known state (channel 0)
    digitalWrite(MUX_S0_PIN, LOW);
    digitalWrite(MUX_S1_PIN, LOW);
    digitalWrite(MUX_S2_PIN, LOW);
    digitalWrite(MUX_S3_PIN, LOW);
    delay(50);
    
    int directRead = analogRead(MUX_SIG_PIN);
    Serial.println("Direct read: " + String(directRead) + " (" + String((directRead * 5.0) / 1023.0, 2) + "V)");
    
    if (directRead == 0 || directRead == 1023) {
        Serial.println("WARNING: Direct read shows extreme value. Possible short circuit!");
        Serial.println("Check MUX SIG pin wiring and connections.");
    }

    // Read a few channels to see the pattern
    Serial.println("\n=== Channel Scan ===");
    Serial.println("Scanning all 16 channels (if chip gets hot, disconnect power!):");
    
    // Read a few channels to see the pattern
    Serial.println("\n=== Channel Scan ===");
    for (int ch = 0; ch < 16; ch++) {
        selectMuxChannel(ch);
        delay(MUX_SETTLE_DELAY_MS);
        int reading = analogRead(MUX_SIG_PIN);
        Serial.println("Channel " + String(ch) + ": " + String(reading) + " (" + String((reading * 5.0) / 1023.0, 2) + "V)");

        // Highlight our test channels
        if (ch == MUX_TEST_GND_CH) {
            Serial.println("  ^ This should be ~0 (GND test channel)");
        }
        if (ch == MUX_TEST_VCC_CH) {
            Serial.println("  ^ This should be ~1023 (VCC test channel)");
        }
    }

    Serial.println("\n=== Performing Standard Tests ===");

    // Test 1: Check GND channel. Expect a value very close to 0.
    Serial.print("Selecting MUX channel " + String(MUX_TEST_GND_CH) + " for GND test... ");
    selectMuxChannel(MUX_TEST_GND_CH);
    delay(MUX_SETTLE_DELAY_MS);
    int gndValue = analogRead(MUX_SIG_PIN);

    bool gndTestPassed = (gndValue < 20); // Allow for a small amount of noise
    if (!gndTestPassed) {
        Serial.print("MUX GND test FAILED. Expected < 20, got: ");
        Serial.println(gndValue);
        Serial.println("TROUBLESHOOTING: Check that MUX channel " + String(MUX_TEST_GND_CH) + " is properly connected to GND.");
    } else {
        Serial.print("OK. Read: ");
        Serial.println(gndValue);
    }

    // Test 2: Check VCC channel. Expect a value very close to 1023.
    Serial.print("Selecting MUX channel " + String(MUX_TEST_VCC_CH) + " for VCC test... ");
    selectMuxChannel(MUX_TEST_VCC_CH);
    delay(MUX_SETTLE_DELAY_MS);
    int vccValue = analogRead(MUX_SIG_PIN);

    bool vccTestPassed = (vccValue > 1000); // Allow for a small voltage drop
    if (!vccTestPassed) {
        Serial.print("MUX VCC test FAILED. Expected > 1000, got: ");
        Serial.println(vccValue);
        // Add a more specific hint if the VCC reading looks like a GND reading
        if (vccValue < 20) {
            Serial.println("HINT: The VCC test channel is reading a ground-level voltage. Please verify that MUX channel " + String(MUX_TEST_VCC_CH) + " is connected to 5V and not to GND or another pin.");
        } else if (vccValue < 500) {
            Serial.println("HINT: Low voltage detected. Check VCC connection to MUX channel " + String(MUX_TEST_VCC_CH) + " and MUX power supply.");
        }
        Serial.println("TROUBLESHOOTING: Check that MUX channel " + String(MUX_TEST_VCC_CH) + " is properly connected to 5V/VCC.");
    } else {
        Serial.print("OK. Read: ");
        Serial.println(vccValue);
    }

    // Enhanced diagnostics based on the readings
    Serial.println("\n=== Diagnostic Analysis ===");

    // Check if both readings are in a similar mid-range (suggests floating inputs)
    if (gndValue > 100 && vccValue < 900 && abs(gndValue - vccValue) < 200) {
        Serial.println("WARNING: Both test channels show mid-range values. This suggests:");
        Serial.println("  1. Test channels may not be properly connected");
        Serial.println("  2. MUX may not be switching channels correctly");
        Serial.println("  3. MUX control pins may be incorrectly wired");
        Serial.println("  4. MUX power supply issues");
    }

    // Check for specific wiring issues
    if (gndValue > 50 && vccValue > 50) {
        Serial.println("POSSIBLE ISSUES:");
        Serial.println("  - MUX Enable pin: Ensure EN pin is connected to GND (not floating)");
        Serial.println("  - MUX Power: Verify VCC pin has 5V and GND pin is connected");
        Serial.println("  - Control Pins: Verify S0-S3 pins are correctly connected");
        Serial.println("  - Test Wiring: Verify channel " + String(MUX_TEST_GND_CH) + " → GND and channel " + String(MUX_TEST_VCC_CH) + " → 5V");
    }

    // Check for power issues after both tests are complete
    if (!gndTestPassed && !vccTestPassed) {
        if (gndValue > 50 && vccValue < 500) {
            Serial.println("\nERROR: Possible POWER ISSUE WITH THE MULTIPLEXER.");
            Serial.println("Both GND and VCC tests are failing, try these steps:");
            Serial.println("    1. Verify that the MUX VCC pin is connected to 5V.");
            Serial.println("    2. Verify that the MUX GND pin is connected to GND.");
            Serial.println("    3. Verify that the MUX EN pin is connected to GND (not floating).");
            Serial.println("    4. Replace the jumper wires used for connecting the MUX board.");
            Serial.println("    5. Check that test channels are properly wired:");
            Serial.println("       - Channel " + String(MUX_TEST_GND_CH) + " should connect to GND");
            Serial.println("       - Channel " + String(MUX_TEST_VCC_CH) + " should connect to 5V");
        }
    }

    if (gndTestPassed && vccTestPassed) {
        Serial.println("Multiplexer validation PASSED.");
        return true;
    } else {
        Serial.println("Multiplexer validation FAILED. Check wiring, power, and MUX_TEST pins in config.h.");
        Serial.println("\nNext steps:");
        Serial.println("1. Verify physical connections based on the diagnostic output above");
        Serial.println("2. Use a multimeter to verify voltages at test channels");
        Serial.println("3. Check MUX datasheet for proper wiring");
        return false;
    }
}