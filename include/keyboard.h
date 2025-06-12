#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define PS2_DATA    0x60
#define PS2_STATUS  0x64
#define PS2_CMD     0x64


#define PS2_RESPONSE_ACK 0xFA
#define PS2_RESPONSE_RES 0xFE


void PS2_init();

// TODO
#endif