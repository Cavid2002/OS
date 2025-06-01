#ifndef VGA_H
#define VGA_H

#include <stdint.h>

uint16_t entry(uint8_t color, uint8_t c);
uint32_t strlen(const char* str);

void putchar(uint8_t c);
void terminal_init();
void terminal_write(const char* str, uint32_t len);
void terminal_puts(const char* str);
void terminal_clean();

#endif