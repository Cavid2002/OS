#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_CMD     0x64

#define PS2_DEVICE_KEYBOARD     0
#define PS2_DEVICE_MOUSE        1
#define PS2_DEVICE_STDMOUSE     2
#define PS2_DEVICE_MOUSEV       3
#define PS2_DEVICE_NCDKEYBOARD  4
#define PS2_DEVICE_UNKNOWN      -1

#define PS2_RESPONSE_ACK 0xFA
#define PS2_RESPONSE_RES 0xFE


#define PS2_TIMEOUT 10000

#define KEYBOARD_BUFF_SIZE 256

int ps2_init();
int ps2_device_identify();

void keyboard_interrupt_routine();

// TODO
#endif