// config.h

// pin‐out pentru cele 3 pompe
const int PUMP1_PIN = 2;
const int PUMP2_PIN = 3;
const int PUMP3_PIN = 4;
const int TRAP_IN1       = 12;
const int TRAP_IN2       = 13;

// prototipuri pentru funcţiile de trapă şi uşă
void openTrap();
void closeTrap();
void openDoor();
void closeDoor();
extern void openTrap();
extern void closeTrap();

// portul webserver‐ului
#define WEB_SERVER_PORT 80