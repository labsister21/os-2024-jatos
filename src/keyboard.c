#include "header/driver/keyboard.h"
#include "header/text/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"

struct KeyboardDriverState keyboard_state;

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

const char keyboard_scancode_shift_to_ascii_map[256] = {
      0, 0x1B, '!', '@', '#', '$', '%', '^',  '&', '*', '(',  ')',  '_', '+', '\b', '\t',
    'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',  'O', 'P', '{',  '}', '\n',   0,  'A',  'S',
    'D',  'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '`',   0, '\\',  'Z', 'X',  'C',  'V',
    'B',  'N', 'M', '<', '>', '?',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

const char keyboard_scancode_capslock_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',  'O', 'P', '[',  ']', '\n',   0,  'A',  'S',
    'D',  'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',   0, '\\',  'Z', 'X',  'C',  'V',
    'B',  'N', 'M', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

const char keyboard_scancode_shift_capslock_to_ascii_map[256] = {
      0, 0x1B, '!', '@', '#', '$', '%', '^',  '&', '*', '(',  ')',  '_', '+', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '{',  '}', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', '<', '>', '?',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

void keyboard_isr(void) {
    uint8_t scancode = in(KEYBOARD_DATA_PORT);
    static bool isShift = false;
    static bool isCapsLock = false;
    static int maxCol = -1;
    static int latestMaxCol = 0;
    static int maxRow = 0;
    // TODO : Implement scancode processing
    // if(keyboard_state.keyboard_input_on){
    //   keyboard_state.keyboard_buffer = keyboard_scancode_1_to_ascii_map[scancode];
    //   pic_ack(1);
    // }

    // Memeriksa apakah scancode adalah capslock
    if ((scancode == 0x3a) && (!isCapsLock)){ // Capslock on
        isCapsLock = true;
        pic_ack(1);
        return;
    } else if ((scancode == 0x3a) && (isCapsLock)){ // Capslock off
        isCapsLock = false;
        pic_ack(1);
        return;
    }

    if (((scancode == 0x2a) || (scancode == 0x36)) && (!isShift)){ // Memeriksa apakah scancode adalah shift hold
        isShift = true;
        pic_ack(1);
        return;
    } else if (((scancode == 0xaa) || (scancode == 0xb6)) && (isShift)){ // Memeriksa apakah scancode adalah shift release
        isShift = false;
        pic_ack(1);
        return;
    }

    struct Cursor c = framebuffer_get_cursor();

    if (keyboard_state.keyboard_input_on) {
        // Memeriksa apakah scancode merupakan tombol "delete" / "left arrow key" / "right arrow key"
        if (scancode == 0xe) { // backspace
            keyboard_state.keyboard_buffer = 0x0;
            if ((c.col > 0) && (c.col <= 79)){
                framebuffer_write(c.row, c.col-1, keyboard_state.keyboard_buffer, 0xF, 0x0);
                if (c.col > 0){
                    c.col--;
                    if (c.col >= maxCol){
                        maxCol--;
                    }
                }
            } else {
                if (c.row > 0){
                    c.row--;
                    c.col = 79;
                    framebuffer_write(c.row, c.col, keyboard_state.keyboard_buffer, 0xF, 0x0);
                }
            }
        } else if (scancode == 0x4b){ // left arrow
            keyboard_state.keyboard_buffer = 0x1F;
            if (c.col > 0){
                c.col--;
            } else if ((c.col == 0) && (c.row > 0)){
                c.row--;
                c.col = 79;
                if (maxCol != 79){
                    latestMaxCol = maxCol;
                }
                maxCol = 79;
            }
        } else if (scancode == 0x4d){ // right arrow
            keyboard_state.keyboard_buffer = 0x1E;
            if (c.col <= maxCol){
                c.col++;
            } else if (c.col == 79){
                c.row++;
                c.col = 0;
                if (c.row == maxRow){
                    maxCol = latestMaxCol;
                }
            }
        } else {
            // Memasukkan karakter sesuai dengan map scancode to ASCII ke dalam buffer
            if ((isShift) && (!isCapsLock)){ // Jika shift tetapi bukan capslock
                if (keyboard_scancode_shift_to_ascii_map[scancode] != 0){
                    keyboard_state.keyboard_buffer = keyboard_scancode_shift_to_ascii_map[scancode];
                }
            } else if ((!isShift) && (isCapsLock)){ // Jika bukan shift tetapi capslock
                if (keyboard_scancode_capslock_to_ascii_map[scancode] != 0){
                    keyboard_state.keyboard_buffer = keyboard_scancode_capslock_to_ascii_map[scancode];
                }
            } else if ((isShift) && (isCapsLock)){ // Jika shift dan capslock
                if (keyboard_scancode_shift_capslock_to_ascii_map[scancode] != 0){
                    keyboard_state.keyboard_buffer = keyboard_scancode_shift_capslock_to_ascii_map[scancode];
                }
            } else { // Jika bukan shift dan bukan capslock.
                if (keyboard_scancode_1_to_ascii_map[scancode] != 0){
                    keyboard_state.keyboard_buffer = keyboard_scancode_1_to_ascii_map[scancode];
                }
            }
            if (keyboard_state.keyboard_buffer != 0x0){
                framebuffer_write(c.row, c.col, keyboard_state.keyboard_buffer, 0xF, 0x0);
                framebuffer_write(c.row, c.col+1, 0, 0xF, 0);
                if (c.col == maxCol + 1){
                    maxCol++;
                }
                c.col++;
                if (c.col >= 80){
                    c.col = 0;
                    c.row++;
                    if (c.row > maxRow){
                        maxRow = c.row;
                    }
                    maxCol = 0;
                }
            }
        }
        framebuffer_set_cursor(c.row, c.col);
        
        // Mengakui interrupt dari PIC
        pic_ack(1);
    }
    
}


void keyboard_state_activate(void) {
    keyboard_state.keyboard_input_on = true;
}

void keyboard_state_deactivate(void) {
    keyboard_state.keyboard_input_on = false;
}

void get_keyboard_buffer(char *buf) {
    *buf = keyboard_state.keyboard_buffer;
    keyboard_state.keyboard_buffer = 0;
}

    