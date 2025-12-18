#ifndef ATAPIO_H
#define ATAPIO_H


#include <stdint.h>

#define TIMEOUT                 1000
#define ATAPIO_PRI              1
#define ATAPIO_SEC              2

#define ATAPIO_PRI_IO_BASE      0x1F0
#define ATAPIO_PRI_CTRL_BASE    0x3F6

#define ATAPIO_SEC_IO_BASE      0x170
#define ATAPIO_SEC_CTRL_BASE    0x376


#define ATAPIO_REG_DATA         0
#define ATAPIO_REG_ERR          1
#define ATAPIO_REG_FEAT         1
#define ATAPIO_REG_SEC_COUNT    2
#define ATAPIO_REG_SEC_NUM      3
#define ATAPIO_REG_LBA_LOW      3
#define ATAPIO_REG_CYL_LOW      4
#define ATAPIO_REG_LBA_MID      4
#define ATAPIO_REG_CYL_HIGH     5
#define ATAPIO_REG_LBA_HIGH     5
#define ATAPIO_REG_SELECT       6
#define ATAPIO_REG_STATUS       7
#define ATAPIO_REG_CMD          7

#define ATAPIO_REG_ALTR_STATUS  0
#define ATAPIO_REG_CTRL         0
#define ATAPIO_REG_DRIVE_ADDR   1


#define ATAPIO_ERR_AMNF     0x01
#define ATAPIO_ERR_TKZNF    0x02
#define ATAPIO_ERR_ABRT     0x04
#define ATAPIO_ERR_MCR      0x08
#define ATAPIO_ERR_IDNF     0x10
#define ATAPIO_ERR_MC       0x20
#define ATAPIO_ERR_UNC      0x40
#define ATAPIO_ERR_BBK      0x80

#define ATAPIO_STATUS_ERR   0x01
#define ATAPIO_STATUS_IDX   0x02
#define ATAPIO_STATUS_CORR  0x04
#define ATAPIO_STATUS_DRQ   0x08
#define ATAPIO_STATUS_SRV   0x10
#define ATAPIO_STATUS_DF    0x20
#define ATAPIO_STATUS_RDY   0x40
#define ATAPIO_STATUS_BSY   0x80

#define ATAPIO_DEV_TYPE_ATA     0x01
#define ATAPIO_DEV_TYPE_ATAPI   0x08

#define ATAPIO_CMD_QUEUE_SIZE   20

typedef struct
{
    uint32_t lba;
    void* buff;
    uint8_t sector_count;
} disk_packet_lba28;


typedef struct
{
    uint32_t lba_low;
    uint16_t lba_high;
    void* buff;
    uint8_t sector_count;
} disk_packet_lba48;


typedef struct
{
    uint16_t io_base;
    uint16_t ctrl_base;
    uint8_t dev_type;
} atapio_bus_regbase;


typedef struct
{
    disk_packet_lba28 pack;
    uint8_t cmd;
} disk_cmd_entry;

uint16_t get_identify_data(uint8_t index);
void atapio_setup_address();
int atapio_wait(uint8_t flag, uint16_t timeout);
int atapio_bus_set();
void atapio_select(uint8_t bus_num, uint8_t drive_num);
int atapio_flush_cache();
void atapio_software_reset(uint8_t bus_num);
int atapio_identify(disk_packet_lba28* pack);
uint8_t atapio_get_status();
int atapio_read_lba28(disk_packet_lba28* pack); 
int atapio_write_lba28(disk_packet_lba28* pack);
int atapio_init();
#endif