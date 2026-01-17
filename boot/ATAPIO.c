#include <stdint.h>
#include "../include/ATAPIO.h"
#include "../include/portio.h"
#include "../include/delay.h"
#include "../include/VGA.h"

static atapio_bus_regbase ata_bus[2];
static uint8_t cur_bus;
static uint8_t cur_drive;
static uint16_t cur_identify_buff[256];

static disk_cmd_entry cmd_queue[20];
static uint8_t cmd_queue_tail = 0;
static uint8_t cmd_queue_head = 0; 


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
                terminal_printf("[DISK ERROR]Abort\n");
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

    if(atapio_wait(ATAPIO_STATUS_BSY, TIMEOUT) < 0)
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
        ata_bus[cur_bus].dev_type = ATAPIO_DEV_TYPE_ATA << (4 * cur_drive);
        return ATAPIO_DEV_TYPE_ATA;
    }

    if(mid == 0x14 && high == 0xEB)
    {
        terminal_printf("[SUCCESS] ATAPI detected\n");
        ata_bus[cur_bus].dev_type = ATAPIO_DEV_TYPE_ATAPI << (4 * cur_drive);
        return ATAPIO_DEV_TYPE_ATAPI;
    }

    terminal_printf("[ATAPIO] SOMETHING IS WRONG\n");
    return -1;
}

void atapio_select(uint8_t bus_num, uint8_t drive_num)
{
    if(cur_bus == bus_num && cur_drive == drive_num) return;
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
    if(atapio_wait(ATAPIO_STATUS_RDY, TIMEOUT) <= 0)   
    {
        terminal_printf("[ERROR]ATAPIO FLUSH TIMEOUT\n");
        return -1;
    }
    return 1;
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
    uint8_t cmd = 0xEC;
    if(ata_bus[cur_bus].dev_type & ATAPIO_DEV_TYPE_ATAPI << (4 * cur_drive))
    {
        terminal_printf("PACKET INTERFACE DETECTED");
        cmd = 0xA1;
    }

    uint16_t ret = 0;
    uint16_t* buff = pack->buff;
    uint16_t port = ata_bus[cur_bus].io_base;
    uint8_t status = 0;
    out_byte(port + ATAPIO_REG_LBA_LOW, 0x00);
    out_byte(port + ATAPIO_REG_LBA_MID, 0x00);
    out_byte(port + ATAPIO_REG_LBA_HIGH, 0x00);

    out_word(port + ATAPIO_REG_CMD, cmd);
    
    if(atapio_wait(ATAPIO_STATUS_DRQ, TIMEOUT) < 0)
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
    if (atapio_wait(ATAPIO_STATUS_RDY, TIMEOUT) < 0)
    {
        terminal_printf("[ERROR] ATAPIO READ ERROR\n");
        return -1;
    }
        

    uint32_t lba = pack->lba;
    uint16_t* buff = (uint16_t*)pack->buff;
    uint8_t sector_count = pack->sector_count;

    uint16_t port = ata_bus[cur_bus].io_base;

    uint8_t head = 0xE0 | ((lba >> 24) & 0x0F);

    out_byte(port + ATAPIO_REG_SELECT, head);

    out_byte(port + ATAPIO_REG_SEC_COUNT, sector_count);
    out_byte(port + ATAPIO_REG_LBA_LOW,  lba & 0xFF);
    out_byte(port + ATAPIO_REG_LBA_MID,  (lba >> 8) & 0xFF);
    out_byte(port + ATAPIO_REG_LBA_HIGH, (lba >> 16) & 0xFF);

    out_byte(port + ATAPIO_REG_CMD, 0x20);

    if (atapio_wait(ATAPIO_STATUS_DRQ, TIMEOUT) < 0)
        return -1;

    for (int s = 0; s < sector_count; s++)
    {
        for (int i = 0; i < 256; i++)
            buff[s * 256 + i] = in_word(port + ATAPIO_REG_DATA);

        if (s != sector_count - 1 && atapio_wait(ATAPIO_STATUS_DRQ, TIMEOUT) < 0)
        {
            terminal_printf("[ERROR] ATAPIO READ ERROR\n");    
            return -1;
        }
                
    }

    return sector_count * 512;  
}



int atapio_write_lba28(disk_packet_lba28* pack)
{
    if (atapio_wait(ATAPIO_STATUS_RDY, TIMEOUT) < 0)
    {
        terminal_printf("[ERROR] ATAPIO WRITE ERROR: drive not ready\n");
        return -1;
    }

    uint32_t lba = pack->lba;
    uint16_t* buff = (uint16_t*)pack->buff;
    uint8_t sector_count = pack->sector_count;

    uint16_t port = ata_bus[cur_bus].io_base;

    uint8_t head = 0xE0 | ((lba >> 24) & 0x0F);

    out_byte(port + ATAPIO_REG_SELECT, head);
    out_byte(port + ATAPIO_REG_SEC_COUNT, sector_count);
    out_byte(port + ATAPIO_REG_LBA_LOW,  lba & 0xFF);
    out_byte(port + ATAPIO_REG_LBA_MID,  (lba >> 8) & 0xFF);
    out_byte(port + ATAPIO_REG_LBA_HIGH, (lba >> 16) & 0xFF);
    out_byte(port + ATAPIO_REG_CMD, 0x30);

    if (atapio_wait(ATAPIO_STATUS_DRQ, TIMEOUT) < 0)
    {
        terminal_printf("[ERROR] ATAPIO WRITE ERROR: no DRQ after command\n");
        return -1;
    }

    for (int s = 0; s < sector_count; s++)
    {
        for (int i = 0; i < 256; i++)
            out_word(port + ATAPIO_REG_DATA, buff[s * 256 + i]);

        if (s != sector_count - 1)
            if (atapio_wait(ATAPIO_STATUS_DRQ, TIMEOUT) < 0)
            {
                terminal_printf("[ERROR] ATAPIO WRITE ERROR: DRQ timeout mid-write\n");
                return -1;
            }
    }

    if (atapio_wait(ATAPIO_STATUS_BSY, TIMEOUT) < 0)
    {
        terminal_printf("[ERROR] ATAPIO WRITE ERROR: BSY timeout after write\n");
        return -1;
    }

    atapio_flush_cache();

    return sector_count * 512;
}

int atapio_init()
{
    if(in_byte(ATAPIO_PRI_IO_BASE + ATAPIO_REG_STATUS) == 0xFF)
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

