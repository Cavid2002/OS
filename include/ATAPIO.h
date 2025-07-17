#ifndef ATAPIO_H
#define ATAPIO_H


#include <stdint.h>


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


typedef struct
{
    uint32_t lba;
    uint16_t* buff;
    uint8_t sector_count;
} disk_packet_lba28;

typedef struct
{
    uint32_t lba_low;
    uint16_t lba_high;
    uint16_t* buff;
    uint8_t sector_count;
} disk_packet_lba48;


typedef struct
{
    uint16_t io_base;
    uint16_t ctrl_base;
} atapio_bus_regbase;

int atapio_flush_cashe();

#endif