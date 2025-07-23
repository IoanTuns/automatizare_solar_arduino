#pragma once
#include <Arduino.h>
#include "config.h"

class DoorControl {
public:
    DoorControl();
    bool isFullyOpen(int doorIndex);
    bool isFullyClosed(int doorIndex);
    void open(int doorIndex);
    void close(int doorIndex);
    void up(int doorIndex);
    void down(int doorIndex);
    void stop(int doorIndex);
};