#include <stdint.h>
#include "../include/ATAPIO.h"
#include "../include/portio.h"
#include "../include/delay.h"
#include "../include/VGA.h"

static atapio_bus_regbase ata_bus[2];
static uint8_t cur_bus;
static uint8_t cur_drive;


void atapio_init()
{
    cur_bus = 0;
    cur_drive = 0;

    ata_bus[0].io_base = ATAPIO_PRI_IO_BASE;
    ata_bus[0].ctrl_base = ATAPIO_PRI_CTRL_BASE;

    ata_bus[1].io_base = ATAPIO_SEC_IO_BASE;
    ata_bus[1].ctrl_base = ATAPIO_SEC_CTRL_BASE;

    atapio_select(0, 0);

    out_word(ATAPIO_PRI_CTRL_BASE + ATAPIO_REG_CTRL, 1 << 2);
    out_word(ATAPIO_SEC_CTRL_BASE + ATAPIO_REG_CTRL, 1 << 2);
    
}

void atapio_select(uint8_t bus_num, uint8_t drive_num)
{
    uint16_t port = ata_bus[bus_num].io_base + ATAPIO_REG_SELECT;
    uint8_t select = in_byte(port);
    select = (select & 0xEF) | (drive_num << 4);
    out_byte(port, select);
    delay_in_ns(420);
}

int atapio_flush_cache()
{
    uint16_t port = ata_bus[cur_bus].io_base + ATAPIO_REG_CMD;
    out_byte(port, 0xE7);
}


void atapio_software_reset(uint8_t bus_num)
{
    uint16_t port = ata_bus[bus_num].ctrl_base + ATAPIO_REG_CTRL;
    uint8_t val = 0b1000110;
    out_byte(port, val);
}

int atapio_identify()
{
    uint16_t port = ata_bus[cur_bus].io_base;
    uint8_t status = 0;
    out_byte(port + ATAPIO_REG_LBA_LOW, 0x00);
    out_byte(port + ATAPIO_REG_LBA_MID, 0x00);
    out_byte(port + ATAPIO_REG_LBA_HIGH, 0x00);

    out_word(port + ATAPIO_REG_CMD, 0xEC);
    
    while(1)
    {
        status = atapio_get_status();
        if(!(status & ATAPIO_STATUS_BSY))
        {
            if(status & ATAPIO_STATUS_DRQ)
            {
                break;
            }
        }
        else if(status & ATAPIO_STATUS_ERR)
        {
            terminal_puts("[ERROR] NON-ATAPI DRIVE!");
            return -1;
        }
    }



    
    

}

uint8_t atapio_get_status()
{
    uint16_t port = ata_bus[cur_bus].ctrl_base + ATAPIO_REG_ALTR_STATUS;
    return in_byte(port);
}

int atapio_check_status(uint16_t status)
{
    if(!(status & ATAPIO_STATUS_BSY) && (status & ATAPIO_STATUS_DRQ))
    {
        return 0;
    }

    if((status & ATAPIO_STATUS_ERR) && (status & ATAPIO_STATUS_DF))
    {
        return -1;
    }
    return 1;
}

int ata_pio_read_lba28(uint8_t drive_num, disk_packet_lba28* pack)
{

}

