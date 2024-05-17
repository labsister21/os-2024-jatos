#include "header/cmos/cmos.h"

Time time;

void init_cmos(){
    // read_cmos();
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

uint8_t bcd_to_binary(uint8_t bcd) {
    return (bcd & 0x0F) + ((bcd / 16) * 10);
}

void read_cmos(char* time_array){
    while(is_update_cmos());

    time.second = bcd_to_binary(get_cmos_reg(REG_SECONDS));
    time.minute = bcd_to_binary(get_cmos_reg(REG_MINUTES));
    time.hour = get_cmos_reg(REG_HOURS);

    uint8_t regB = get_cmos_reg(0x0B);

    // // Convert BCD to binary if necessary
    // if (!(regB & 0x04)) {
    //     time.hour = bcd_to_binary(time.hour);
    // }

    if (!(regB & 0x04)) {
        time.second = (time.second & 0x0F) + ((time.second / 16) * 10);
        // time.minute = ((time.minute & 0x0F) + ((time.minute / 16) * 10));
        time.hour = (( (time.hour & 0x0F) + (((time.hour & 0x70) / 16) * 10) ) | (time.hour & 0x80)) + 7 % 24;
    }

    time_array[0] = numToStrLeft(time.hour);
    time_array[1] = numToStrRight(time.hour);
    time_array[2] = numToStrLeft(time.minute);
    time_array[3] = numToStrRight(time.minute);
    time_array[4] = numToStrLeft(time.second);
    time_array[5] = numToStrRight(time.second);
}

char numToStrLeft(int num) {

    char str[3];
    if (num < 10) {
        str[0] = '0';
        str[1] = num | '0';
    } else {
        int tens = num / 10;
        int units = num % 10;
        str[0] = tens| '0';
        str[1] = units | '0';
    }

    return str[0];
}

char numToStrRight(int num) {

    char str[3];
    if (num < 10) {
        str[0] = '0';
        str[1] = num | '0';
    } else {
        int tens = num / 10;
        int units = num % 10;
        str[0] = tens| '0';
        str[1] = units | '0';
    }

    return str[1];
}