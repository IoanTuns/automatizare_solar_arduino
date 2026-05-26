#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <LiquidCrystal_I2C.h>

class LcdDisplay {
public:
    LcdDisplay(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows);
    void init();
    void clear();
    void print(const char* message);
    void setCursor(uint8_t col, uint8_t row);
    void printAt(uint8_t col, uint8_t row, const char* message);

private:
    LiquidCrystal_I2C _lcd;
    uint8_t _lcd_addr;
    uint8_t _lcd_cols;
    uint8_t _lcd_rows;
};

#endif // LCDDISPLAY_H
