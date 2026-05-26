#include "LcdDisplay.h"

LcdDisplay::LcdDisplay(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows)
    : _lcd(lcd_addr, lcd_cols, lcd_rows), _lcd_addr(lcd_addr), _lcd_cols(lcd_cols), _lcd_rows(lcd_rows) {
}

void LcdDisplay::init() {
    _lcd.init();
    _lcd.backlight();
}

void LcdDisplay::clear() {
    _lcd.clear();
}

void LcdDisplay::print(const char* message) {
    _lcd.print(message);
}

void LcdDisplay::setCursor(uint8_t col, uint8_t row) {
    _lcd.setCursor(col, row);
}

void LcdDisplay::printAt(uint8_t col, uint8_t row, const char* message) {
    setCursor(col, row);
    print(message);
}
