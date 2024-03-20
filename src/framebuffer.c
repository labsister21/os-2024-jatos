#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    uint16_t position = r * 80 + c;

    out(CURSOR_PORT_CMD, 0x0F);
    out(CURSOR_PORT_DATA, (uint8_t)(position & 0xFF));
    out(CURSOR_PORT_CMD, 0x0E);
    out(CURSOR_PORT_DATA, (uint8_t)((position >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    FRAMEBUFFER_MEMORY_OFFSET[(row * 80 + col) * 2] = c;
    FRAMEBUFFER_MEMORY_OFFSET[(row * 80 + col) * 2 + 1] = (bg  << 4) | (fg & 0x0F); 
} 

void framebuffer_clear(void) {
    // for(int i = 0 ;i < 25;i++){
    //     for(int j = 0;j < 80;j++){
    //         framebuffer_write(i,j,0x0,0x7,0x0);
    //     }
    // }
    memset(FRAMEBUFFER_MEMORY_OFFSET,0 ,80 * 25 *2);
}