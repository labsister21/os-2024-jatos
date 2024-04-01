#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"
#include "header/cpu/gdt.h"
#include "header/interrupt/idt.h"
#include "header/interrupt/interrupt.h"
#include "header/driver/keyboard.h"

void kernel_setup(void)
{
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);

    int col = -1;
    int row = 0;
    keyboard_state_activate();
    framebuffer_write(0, 0, 0x0, 0x7, 0x0);

    while (true)
    {   

        char c;
        get_keyboard_buffer(&c);

        /* backspace */
        if (c == 0xE){
            if (col > -1){
                framebuffer_write(row, col, 0x0, 0x7, 0x0);
                col--;
                if (col == -1){
                    framebuffer_set_cursor(0, 0);
                } else {
                    framebuffer_set_cursor(0, col);
                }
            }
        }
        
        else if(c){
            col++;
            framebuffer_write(0, col, c, 0xF, 0);
            framebuffer_set_cursor(0, col);
        }
    }
}