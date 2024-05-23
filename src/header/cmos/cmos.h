#ifndef _CMOS_H
#define _CMOS_H

#include "header/cpu/portio.h"
#include "header/interrupt/interrupt.h"

#define CURRENT_YEAR 2023                                    
#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

#define REG_SECONDS 0x00
#define REG_MINUTES 0x02
#define REG_HOURS 0x04  

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
} Time;

// extern Time time;

void init_cmos();

int8_t is_update_cmos();

uint8_t get_cmos_reg(int reg);

void set_cmos_reg(int reg, uint8_t value);

uint8_t bcd_to_binary(uint8_t bcd);
      
void read_cmos(char* time_array);

char numToStrLeft(int num);
char numToStrRight(int num);

#endif