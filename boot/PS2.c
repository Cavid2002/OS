#include "../include/PS2.h"
#include "../include/portio.h"
#include "../include/interrupt.h"
#include "../include/VGA.h"
#include "../include/PIC.h"

static  uint8_t ps2_dual_channel_precense = 1;

static uint8_t keyboar_buff[KEYBOARD_BUFF_SIZE];
static uint8_t read_ptr = 0;
static uint8_t write_ptr = 0;
static uint8_t cur_device = 0;


void keyboard_interrupt_routine()
{
    terminal_puts("KEYBOARD INTERRUPT");
    in_byte(0x60);
    PIC_send_eoi(1);
}


void ps2_flush_output_buffer()
{
    while(in_byte(PS2_STATUS) & 0x01) 
    {
        in_byte(PS2_DATA);
    }
}

uint8_t ps2_output_wait(uint16_t timeout)
{
    uint8_t status;
    while(timeout)
    {
        status = in_byte(PS2_STATUS);
        if(status & 0x01) return status;
        timeout--;
    }

    terminal_printf("Output Wait time out!\n");
    return 0xFF;
}


uint8_t ps2_input_wait(uint16_t timeout)
{
    uint8_t status;
    while(timeout)
    {
        status = in_byte(PS2_STATUS);
        if(!(status & 0x02)) return status;
        timeout--;
    }

    terminal_printf("Input Wait time out!\n");
    return 0xFF;
}

void ps2_write_data(uint8_t data)
{
    ps2_input_wait(PS2_TIMEOUT);
    out_byte(PS2_DATA, data);
}

void ps2_write_cmd(uint8_t cmd)
{
    ps2_input_wait(PS2_TIMEOUT);
    out_byte(PS2_CMD, cmd);
}

uint8_t ps2_read_data()
{
    uint8_t status = ps2_output_wait(PS2_TIMEOUT);
    if(status == 0xFF)
    {
        return 0xFF;
    }
    uint8_t ret = in_byte(PS2_DATA);
    return ret;
}

void ps2_send_data_port2(uint8_t data)
{
    ps2_write_cmd(0xD4);
    ps2_write_data(data);
    
}


int ps2_init()
{
    disable_interrupt();

    ps2_write_cmd(0xAD);
    ps2_write_cmd(0xA7);
    
    ps2_flush_output_buffer();
    

    ps2_write_cmd(0x20);
    uint8_t config = ps2_read_data();
    if(config != 0xFF && config & 0x04 == 0)
    {
        terminal_printf("POST Test failed error\n");
        return -1;
    }
    terminal_printf("POST Test Success!\n");
    config &= 0b00100110;
    ps2_write_cmd(0x60);
    ps2_write_data(config);


    ps2_write_cmd(0xAA);
    if(ps2_read_data() != 0x55)
    {
        terminal_printf("PS/2 Controller ERROR\n");
        return -1;
    }

    ps2_write_cmd(0xAB);
    if(ps2_read_data() != 0x00)
    {
        terminal_printf("PS/2 Port ERROR\n");
        return -1;
    }
    
    ps2_write_cmd(0x60);
    ps2_write_data(config);
    
    ps2_write_cmd(0xA8);
    ps2_write_cmd(0x20);
    if(ps2_read_data() & 0x20)
    {
        terminal_printf("PS/2 is not dual channel\n");
        ps2_dual_channel_precense = 0;
    }
    

    ps2_write_cmd(0x20);
    config = ps2_read_data();
    ps2_write_cmd(0xAE);
    if(ps2_dual_channel_precense == 1)
    {
        ps2_write_cmd(0xA8);    
        config |= 0x03;
    }
    else
    {
        config |= 0x01;
    }

    ps2_write_cmd(0x60);
    ps2_write_data(config);

    ps2_device_identify();

    ps2_write_data(0xFF);
    if(ps2_read_data() != 0xFA)
    {
        terminal_printf("Reset error!");
        return -1;
    }

    enable_interrupt();
    terminal_printf("PS2 is initialized!\n");
    return 0;
}


int ps2_device_identify()
{
    uint16_t dev = 0xFFFF;
    ps2_write_data(0xF5);
    if(ps2_read_data() != 0xFA)
    {
        terminal_printf("Disable Scanning Error\n");
        return -1;
    }

    ps2_write_data(0xF2);
    if(ps2_read_data() != 0xFA)
    {
        terminal_printf("PS2 Identify Error\n");
        return -1;
    }

    dev = ps2_read_data();
    dev = (dev << 8) | ps2_read_data();
    
    ps2_write_data(0xF4);
    
    switch (dev)
    {
    case 0x00FF:
        terminal_printf("MOUSE Detected\n");
        return PS2_DEVICE_MOUSE;
    case 0xAB83: case 0xABC1:
        terminal_printf("KEYBOARD Detected\n");
        return PS2_DEVICE_KEYBOARD;
    case 0x0300:
        terminal_printf("STD MOUSE Detected\n");
        return PS2_DEVICE_STDMOUSE;
    case 0xAB85:
        terminal_printf("NCD KEYBOARD Detected\n");
        return PS2_DEVICE_NCDKEYBOARD;
    default:
        terminal_printf("UNKNOWN DEVICE Detected\n");
        return PS2_DEVICE_UNKNOWN;
    }
}   


void keyboard()
{

}


//TODO