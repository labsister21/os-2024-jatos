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
    if (framebuffer_get_cursor().col != 0){
        maxCol = framebuffer_get_cursor().col;
    }
    // static int latestMaxCol = 0;
    static int maxRow = 0;

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

    struct Cursor cursor = framebuffer_get_cursor();

    if (keyboard_state.keyboard_input_on && scancode < 0x80) {
   
        if (scancode == 0xe) { // backspace
            keyboard_state.keyboard_buffer = '\b';
            if ((cursor.col > 0) && (cursor.col <= 79)){
                framebuffer_write(cursor.row, cursor.col-1, 0x0, 0xF, 0x0);
                if (cursor.col > 0){
                    cursor.col--;
                    if (cursor.col >= maxCol){
                        maxCol--;
                    }
                }
            } else {
                if (cursor.row > 0){
                    cursor.row--;
                    cursor.col = 79;
                    framebuffer_write(cursor.row, cursor.col, 0x0, 0xF, 0x0);
                }
            }
        } else if (scancode == 0x4b){ // left arrow
            keyboard_state.keyboard_buffer = 0x0;
            pic_ack(1);
            return;

        } else if (scancode == 0x4d){ // right arrow
            keyboard_state.keyboard_buffer = 0x0;
            pic_ack(1);
            return;
     
        } else if (scancode == 0x1c) {
            keyboard_state.keyboard_buffer = '\n';
            cursor.col = 0;
            cursor.row++;
            if (cursor.row > maxRow){
                maxRow = cursor.row;
            }
            maxCol = 0;
        
        } else {
            // Memasukkan karakter sesuai dengan map scancode to ASCII ke dalam buffer
            if ((isShift) && (!isCapsLock)){ // Jika shift tetapi bukan capslock
                keyboard_state.keyboard_buffer = keyboard_scancode_shift_to_ascii_map[scancode];
            } else if ((!isShift) && (isCapsLock)){ // Jika bukan shift tetapi capslock
                keyboard_state.keyboard_buffer = keyboard_scancode_capslock_to_ascii_map[scancode];
            } else if ((isShift) && (isCapsLock)){ // Jika shift dan capslock
                keyboard_state.keyboard_buffer = keyboard_scancode_shift_capslock_to_ascii_map[scancode];
            } else { // Jika bukan shift dan bukan capslock
                keyboard_state.keyboard_buffer = keyboard_scancode_1_to_ascii_map[scancode];
            }

            if (scancode != 0x0 && keyboard_state.keyboard_buffer == 0x0 ){
                pic_ack(1);
                return;
            }
            
            framebuffer_write(cursor.row, cursor.col, keyboard_state.keyboard_buffer, 0xF, 0x0);
  
            cursor.col++;
            if (cursor.col >= 80){
                cursor.col = 0;
                cursor.row++;
                if (cursor.row > maxRow){
                    maxRow = cursor.row;
                }
                maxCol = 0;
                }

        }
        framebuffer_write(cursor.row, cursor.col, 0x0, 0xF, 0x0);
        framebuffer_set_cursor(cursor.row, cursor.col);
        
        // Mengakui interrupt dari PIC
        pic_ack(1);
    }
    pic_ack(1);
    
}


void keyboard_state_activate(void) {
    keyboard_state.keyboard_input_on = true;
}

void keyboard_state_deactivate(void) {
    keyboard_state.keyboard_input_on = false;
}

void get_keyboard_buffer(char *buf) {
    // *buf = keyboard_state.keyboard_buffer;
    memcpy(buf, (void *) &keyboard_state.keyboard_buffer, 1);
    keyboard_state.keyboard_buffer = 0;
    // framebuffer_write(0, 0, *buf, 0xF, 0x0);
}

    