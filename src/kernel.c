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

    int col = 0;
    int row = 0;
    keyboard_state_activate();
    while (true)
    {
        char c;
        get_keyboard_buffer(&c);
        /* backspace */
        if (c == 0xE){
            framebuffer_write(row, col-1, 0x0,0x7,0x0);
            col--;
        }
        else if(c){
            framebuffer_write(0, col++, c, 0xF, 0);
        }
    }
}