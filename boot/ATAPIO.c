#include <stdint.h>
#include "../include/ATAPIO.h"
#include "../include/portio.h"
#include "../include/delay.h"
#include "../include/VGA.h"

static atapio_bus_regbase ata_bus[2];
static uint8_t cur_bus;
static uint8_t cur_drive;
static uint16_t cur_identify_buff[256];

uint16_t get_identify_data(uint8_t index)
{
    return cur_identify_buff[index];
}

void atapio_setup_address()
{
    terminal_printf("[ATAPIO] SETTING ATAPIO ADDRESSES\n");
    ata_bus[0].io_base = ATAPIO_PRI_IO_BASE;
    ata_bus[0].ctrl_base = ATAPIO_PRI_CTRL_BASE;

    ata_bus[1].io_base = ATAPIO_SEC_IO_BASE;
    ata_bus[1].ctrl_base = ATAPIO_SEC_CTRL_BASE;
}

int atapio_wait(uint8_t flag, uint16_t timeout)
{
    uint8_t status = 0;
    
    while(timeout)
    {
        status = atapio_get_status();
        if(!(status & ATAPIO_STATUS_BSY))
        {
            if(status & (ATAPIO_STATUS_ERR | ATAPIO_STATUS_DF))
            {
                return -1;
            }
            

            if((status & flag) || (flag == ATAPIO_STATUS_BSY))
            {
                return 1;
            }
        }
        delay_in_us(1);
        timeout--;
    }

    terminal_printf("[ERROR] WAIT TIMEOUT!\n");
    return -1;
}

int atapio_bus_set()
{
    terminal_printf("[ATAPIO] SETTING BUSSES\n");
    uint16_t port = ata_bus[cur_bus].ctrl_base;

    out_byte(port + ATAPIO_REG_CTRL, 1 << 2);
    delay_in_us(5);
    
    out_byte(port + ATAPIO_REG_CTRL, 1 << 1);
    delay_in_ms(2);

    if(atapio_wait(ATAPIO_STATUS_BSY, 1000) < 0)
    {
        terminal_printf("[ERROR-ATAPIO] init error\n");
        return -1;
    }
    port = ata_bus[cur_bus].io_base;
    uint8_t error = in_byte(port + ATAPIO_REG_ERR);

    if(in_byte(port + ATAPIO_REG_SEC_COUNT) != 1
        || in_byte(port + ATAPIO_REG_SEC_NUM) != 1)
    {
        return -1;
    }
    
    uint8_t high = in_byte(port + ATAPIO_REG_LBA_HIGH);
    uint8_t mid = in_byte(port + ATAPIO_REG_LBA_MID);
    if(mid == 0 && high == 0)
    {
        terminal_printf("[SUCCESS] ATAPIO detected");
        return ATAPIO_DEV_TYPE_ATA;
    }

    terminal_printf("[ATAPIO] SOMETHING IS WRONG\n");
    return -1;
}

void atapio_select(uint8_t bus_num, uint8_t drive_num)
{
    cur_bus = bus_num;
    cur_drive = drive_num;
    uint16_t port = ata_bus[bus_num].io_base + ATAPIO_REG_SELECT;
    uint8_t select = 0b11100000 | drive_num << 4;
    out_byte(port, select);
    delay_in_ns(420);
}

int atapio_flush_cache()
{
    uint16_t port = ata_bus[cur_bus].io_base + ATAPIO_REG_CMD;
    out_byte(port, 0xE7);
    atapio_wait(ATAPIO_STATUS_BSY, 100);
    return 0;
}


void atapio_software_reset(uint8_t bus_num)
{
    uint16_t port = ata_bus[bus_num].ctrl_base + ATAPIO_REG_CTRL;
    uint8_t val = 0b0000110;
    out_byte(port, val);
    delay_in_ms(1);
    val = 0b00000010;
    out_byte(port, val);
    delay_in_ms(1);
    
}

int atapio_identify(disk_packet_lba28* pack)
{
    terminal_printf("ATAPIO IDENTIFY START...\n");
    uint16_t ret = 0;
    uint16_t* buff = pack->buff;
    uint16_t port = ata_bus[cur_bus].io_base;
    uint8_t status = 0;
    out_byte(port + ATAPIO_REG_LBA_LOW, 0x00);
    out_byte(port + ATAPIO_REG_LBA_MID, 0x00);
    out_byte(port + ATAPIO_REG_LBA_HIGH, 0x00);

    out_word(port + ATAPIO_REG_CMD, 0xEC);
    
    if(atapio_wait(ATAPIO_STATUS_DRQ, 1000) < 0)
    {
        terminal_printf("ATAPIO Read Error!");
        return -1;
    }

    for(int i = 0; atapio_get_status() & ATAPIO_STATUS_DRQ; i++)
    {
        buff[i] = in_byte(port + ATAPIO_REG_DATA);
    }

    terminal_printf("ATAPIO IDENTIFY SUCCESS!\n");
    return ret * 2;
}

uint8_t atapio_get_status()
{
    uint16_t port = ata_bus[cur_bus].ctrl_base + ATAPIO_REG_ALTR_STATUS;
    return in_byte(port);
}


int atapio_read_lba28(disk_packet_lba28* pack)
{
    uint16_t ret = 0;
    uint32_t lba = pack->lba;
    uint16_t* buff = pack->buff;
    uint16_t port = ata_bus[cur_bus].io_base;
    uint8_t val = 0b11100000 | (cur_drive << 4);
    
    
    out_byte(port + ATAPIO_REG_SEC_COUNT, pack->sector_count);
    out_byte(port + ATAPIO_REG_LBA_LOW, lba & 0x000000FF);
    out_byte(port + ATAPIO_REG_LBA_MID, (lba & 0x0000FF00) >> 8);
    out_byte(port + ATAPIO_REG_LBA_HIGH, (lba & 0x00FF0000) >> 16);
    out_byte(port + ATAPIO_REG_SELECT, val | (lba & 0x0F000000) >> 24);
    out_byte(port + ATAPIO_REG_CMD, 0x20);

    if(atapio_wait(ATAPIO_STATUS_DRQ, 1000) < 0)
    {
        terminal_printf("ATAPIO Read Error!");
        return -1;
    }

    for(int i = 0; atapio_get_status() & ATAPIO_STATUS_DRQ; i++)
    {
        buff[i] = in_word(port + ATAPIO_REG_DATA);
        ret++;
    }
    return ret * 2;
}


int atapio_write_lba28(disk_packet_lba28* pack)
{
    uint16_t ret = 0;
    uint32_t lba = pack->lba;
    uint16_t* buff = (uint16_t*)pack->buff;
    uint16_t port = ata_bus[cur_bus].io_base;
    uint8_t val = in_byte(port + ATAPIO_REG_SELECT);
    
    out_byte(port + ATAPIO_REG_SEC_COUNT, pack->sector_count);
    out_byte(port + ATAPIO_REG_LBA_LOW, lba & 0x000000FF);
    out_byte(port + ATAPIO_REG_LBA_MID, (lba & 0x0000FF00) >> 8);
    out_byte(port + ATAPIO_REG_LBA_HIGH, (lba & 0x00FF0000) >> 16);
    out_byte(port + ATAPIO_REG_SELECT, val | ((lba & 0x0F000000) >> 24));
    out_byte(port + ATAPIO_REG_CMD, 0x30);

    if(atapio_wait(ATAPIO_STATUS_DRQ, 1000) < 0)
    {
        terminal_printf("ATAPIO Read Error!");
        return -1;
    }

    for(int i = 0; atapio_get_status() & ATAPIO_STATUS_DRQ; i++)
    {
        out_word(port + ATAPIO_REG_DATA, buff[i]);
        ret++;
    }

    return ret * 2;
}

int atapio_init()
{
    if(in_byte(ata_bus[cur_bus].ctrl_base + ATAPIO_REG_STATUS) == 0xFF)
    {
        terminal_printf("[ERROR] ATA BUS NOT FOUND");
        return -1;
    }
    disk_packet_lba28 pack;
    pack.buff = cur_identify_buff;
    pack.sector_count = 0;
    pack.lba = 0;
    
    atapio_setup_address();
    atapio_select(0, 0);
    atapio_bus_set();
    atapio_identify(&pack);
    
}