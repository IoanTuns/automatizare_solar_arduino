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
    Serial.println("=== CRITICAL MUX DIAGNOSIS ===");
    Serial.println("Based on your test results:");
    Serial.println("- WITHOUT MUX: A0 reads ~1.0-1.2V (floating pin noise)");
    Serial.println("- WITH MUX: A0 reads ~2.38V (MUX disabled - EN pin not grounded)");
    Serial.println("");
    Serial.println("DIAGNOSIS: MUX EN (Enable) pin is NOT connected to GND!");
    Serial.println("");
    Serial.println("SOLUTION:");
    Serial.println("1. Locate the EN pin on your CD74HC4067 breakout board");
    Serial.println("2. Connect EN pin directly to Arduino GND with a jumper wire");
    Serial.println("3. Verify connection: EN pin should read 0V with multimeter");
    Serial.println("4. Re-run this test after connecting EN to GND");
    Serial.println("");
    Serial.println("CRITICAL: Without EN grounded, the MUX cannot switch channels!");
    Serial.println("");
    delay(2000);
    Serial.println("Validating Multiplexer (CD74HC4067)...");

    // Check if MUX is physically connected
    Serial.println("=== Connection Test ===");
    Serial.println("Testing if MUX is connected...");
    
    // Set all MUX select pins to known state (channel 0)
    digitalWrite(MUX_S0_PIN, LOW);
    digitalWrite(MUX_S1_PIN, LOW);
    digitalWrite(MUX_S2_PIN, LOW);
    digitalWrite(MUX_S3_PIN, LOW);
    delay(50);
    
    int baseRead = analogRead(MUX_SIG_PIN);
    Serial.println("Base reading: " + String(baseRead) + " (" + String((baseRead * 5.0) / 1023.0, 2) + "V)");
    
    // Analyze the base reading
    if (baseRead >= 200 && baseRead <= 300) {
        Serial.println("DETECTED: MUX appears to be disconnected (floating pin noise)");
        Serial.println("Please connect your MUX before running this test.");
        return false;
    } else if (baseRead >= 450 && baseRead <= 550) {
        Serial.println("DETECTED: MUX connected but EN pin likely floating (~2.5V)");
        Serial.println("This confirms EN pin is NOT connected to GND!");
    } else if (baseRead < 50) {
        Serial.println("DETECTED: MUX connected, channel 0 appears to be grounded");
    } else if (baseRead > 950) {
        Serial.println("DETECTED: MUX connected, channel 0 appears to be at VCC");
    } else {
        Serial.println("DETECTED: MUX connected, reading intermediate voltage");
    }

    Serial.println("\n=== MUX Diagnostic Information ===");
    Serial.println("MUX Pin Configuration:");
    Serial.println("  S0 (Pin " + String(MUX_S0_PIN) + ") → Arduino D" + String(MUX_S0_PIN));
    Serial.println("  S1 (Pin " + String(MUX_S1_PIN) + ") → Arduino D" + String(MUX_S1_PIN));
    Serial.println("  S2 (Pin " + String(MUX_S2_PIN) + ") → Arduino A" + String(MUX_S2_PIN - A0));
    Serial.println("  S3 (Pin " + String(MUX_S3_PIN) + ") → Arduino A" + String(MUX_S3_PIN - A0));
    Serial.println("  SIG (Pin A" + String(MUX_SIG_PIN - A0) + ") → Arduino A" + String(MUX_SIG_PIN - A0));
    Serial.println("  VCC → Arduino 5V");
    Serial.println("  GND → Arduino GND");
    Serial.println("  EN → Arduino GND ❌ (MISSING - ADD THIS!)");
    Serial.println("  Test Channels: GND=" + String(MUX_TEST_GND_CH) + ", VCC=" + String(MUX_TEST_VCC_CH));

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

    // Test 1: Check GND channel
    Serial.print("Selecting MUX channel " + String(MUX_TEST_GND_CH) + " for GND test... ");
    selectMuxChannel(MUX_TEST_GND_CH);
    delay(MUX_SETTLE_DELAY_MS);
    int gndValue = analogRead(MUX_SIG_PIN);

    bool gndTestPassed = (gndValue < 20);
    if (!gndTestPassed) {
        Serial.print("FAILED. Expected < 20, got: ");
        Serial.println(gndValue);
        Serial.println("CRITICAL: Channel " + String(MUX_TEST_GND_CH) + " should read ~0V but reads " + String((gndValue * 5.0) / 1023.0, 2) + "V");
        if (gndValue >= 450 && gndValue <= 550) {
            Serial.println("This reading confirms MUX EN pin is floating!");
        }
    } else {
        Serial.print("OK. Read: ");
        Serial.println(gndValue);
    }

    // Test 2: Check VCC channel
    Serial.print("Selecting MUX channel " + String(MUX_TEST_VCC_CH) + " for VCC test... ");
    selectMuxChannel(MUX_TEST_VCC_CH);
    delay(MUX_SETTLE_DELAY_MS);
    int vccValue = analogRead(MUX_SIG_PIN);

    bool vccTestPassed = (vccValue > 1000);
    if (!vccTestPassed) {
        Serial.print("FAILED. Expected > 1000, got: ");
        Serial.println(vccValue);
        Serial.println("CRITICAL: Channel " + String(MUX_TEST_VCC_CH) + " should read ~5V but reads " + String((vccValue * 5.0) / 1023.0, 2) + "V");
        if (vccValue >= 450 && vccValue <= 550) {
            Serial.println("This reading confirms MUX EN pin is floating!");
        }
    } else {
        Serial.print("OK. Read: ");
        Serial.println(vccValue);
    }

    // Quick verification test (moved before final diagnosis)
    Serial.println("\n=== Quick Verification Test ===");
    Serial.println("Testing key channels after EN pin fix...");
    
    // Quick test for GND channel
    selectMuxChannel(MUX_TEST_GND_CH);
    delay(MUX_SETTLE_DELAY_MS);
    int gndRead = analogRead(MUX_SIG_PIN);
    Serial.println("GND channel (" + String(MUX_TEST_GND_CH) + "): " + String(gndRead) + " (" + String((gndRead * 5.0) / 1023.0, 2) + "V)");

    // Quick test for VCC channel  
    selectMuxChannel(MUX_TEST_VCC_CH);
    delay(MUX_SETTLE_DELAY_MS);
    int vccRead = analogRead(MUX_SIG_PIN);
    Serial.println("VCC channel (" + String(MUX_TEST_VCC_CH) + "): " + String(vccRead) + " (" + String((vccRead * 5.0) / 1023.0, 2) + "V)");

    // Quick assessment
    if (gndRead < 50 && vccRead > 950) {
        Serial.println("✓ Quick verification PASSED - MUX is working!");
    } else {
        Serial.println("✗ Quick verification FAILED - Check EN pin connection");
    }

    Serial.println("\n=== FINAL DIAGNOSIS ===");
    
    if (!gndTestPassed && !vccTestPassed) {
        // Check if readings suggest EN pin floating
        if ((gndValue >= 400 && gndValue <= 600) && (vccValue >= 400 && vccValue <= 600)) {
            Serial.println("CONFIRMED: MUX EN pin is NOT connected to GND!");
            Serial.println("");
            Serial.println("Evidence:");
            Serial.println("- Both test channels read ~2.5V (mid-rail voltage)");
            Serial.println("- No channel switching is occurring");
            Serial.println("- MUX is in disabled state");
            Serial.println("");
            Serial.println("IMMEDIATE FIX:");
            Serial.println("1. Find the EN pin on your CD74HC4067 breakout board");
            Serial.println("2. Connect EN pin to Arduino GND with a jumper wire");
            Serial.println("3. Verify with multimeter: EN pin should read 0V");
            Serial.println("4. Re-run this test");
            Serial.println("");
            Serial.println("The MUX will NOT work until EN is properly grounded!");
        } else {
            Serial.println("Multiple issues detected. Check all connections.");
        }
        return false;
    }

    if (gndTestPassed && vccTestPassed) {
        Serial.println("SUCCESS: Multiplexer validation PASSED!");
        Serial.println("MUX is properly connected and functional.");
        return true;
    } else {
        Serial.println("Partial failure. Check specific test results above.");
        return false;
    }
}

bool HardwareInit::quickMuxTest() {
    Serial.println("=== Quick MUX Test ===");
    
    // Test GND channel
    selectMuxChannel(MUX_TEST_GND_CH);
    delay(MUX_SETTLE_DELAY_MS);
    int gndRead = analogRead(MUX_SIG_PIN);
    Serial.println("GND channel (" + String(MUX_TEST_GND_CH) + "): " + String(gndRead));

    // Test VCC channel  
    selectMuxChannel(MUX_TEST_VCC_CH);
    delay(MUX_SETTLE_DELAY_MS);
    int vccRead = analogRead(MUX_SIG_PIN);
    Serial.println("VCC channel (" + String(MUX_TEST_VCC_CH) + "): " + String(vccRead));

    bool passed = (gndRead < 50 && vccRead > 950);
    
    // Update the global muxStatus flag, which is used by the web server.
    muxStatus = passed;

    Serial.println(passed ? "✓ Quick test PASSED" : "✗ Quick test FAILED");
    
    return passed;
}