#include "header/cmos/cmos.h"

static Time time;

void init_cmos(){
    read_cmos();
}

int8_t is_update_cmos() {
    out(CMOS_ADDRESS, 0x0A);
    return (in(CMOS_DATA) & 0x80);
}
 
uint8_t get_cmos_reg(int reg) {
    out(CMOS_ADDRESS, reg);
    return in(CMOS_DATA);
}

void set_cmos_reg(int reg, uint8_t value){
    out(CMOS_ADDRESS, reg);
    out(CMOS_DATA, value);
}

void read_cmos(){
    while(is_update_cmos());

    time.second = get_cmos_reg(0x00);
    time.minute = get_cmos_reg(0x02);
    time.hour = get_cmos_reg(0x04);

    uint8_t regB = get_cmos_reg(0x0B);

    if(!(regB & 0x04)){
        time.second = (time.second & 0x0F) + ((time.second / 16) * 10);
        time.minute = (time.minute & 0x0F) + ((time.minute / 16) * 10);
        time.hour = ((time.hour & 0x0F) + (((time.hour & 0x70) / 16) * 10)) | (time.hour & 0x80);
    }
}

void write_cmos(Time * time){
    while (is_update_cmos());

    set_cmos_reg(REG_SECONDS, time->second);
    set_cmos_reg(REG_MINUTES, time->minute);
    set_cmos_reg(REG_HOURS, time->hour);
}