#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

struct Cursor cursor = {0, 0};

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
    memset(FRAMEBUFFER_MEMORY_OFFSET,0 ,80 * 25 *2);
    cursor.col = 0;
    cursor.row = 0;
}



struct Cursor framebuffer_get_cursor() {
    out(CURSOR_PORT_CMD, 0x0E);
    int offset = in(CURSOR_PORT_DATA) << 8;
    out(CURSOR_PORT_CMD, 0x0F);
    offset += in(CURSOR_PORT_DATA);
    struct Cursor c =
    {
        .row = offset / 80,
        .col = offset % 80
    };
    return c;
}


void putchar(char c, char color) {
    framebuffer_write(cursor.row, cursor.col, c, color, 0);
}

void puts(char* str, uint32_t count, char color) {
    for (uint32_t i = 0; i < count; i++) {
        if (str[i] == '\n')  {
            cursor.row++;
            cursor.col = 0;
        } else {
            if (cursor.col == 80) {
                cursor.row++;
                cursor.col = 0;
            }
            putchar(str[i], color);
            cursor.col++;
        }
    }
    framebuffer_write(cursor.row, cursor.col, 0x0, 0xF, 0);
    framebuffer_set_cursor(cursor.row, cursor.col);
}