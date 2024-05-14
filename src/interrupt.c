#include "header/interrupt/interrupt.h"
#include "header/driver/keyboard.h"
#include "header/cpu/portio.h"
#include "header/cpu/gdt.h"
#include "header/filesystem/fat32.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/scheduler/scheduler.h"
#include "header/cmos/cmos.h"

char* numbers[17] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"}; 


void activate_timer_interrupt(void) {
    __asm__ volatile("cli");
    // Setup how often PIT fire
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) (pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) ((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Activate the interrupt
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));
}



void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}



void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

struct TSSEntry _interrupt_tss_entry = {
    .ss0  = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}


void syscall(struct InterruptFrame frame) {
    switch (frame.cpu.general.eax) {

        case 0:
        /*read*/
            *((int8_t*) frame.cpu.general.ecx) = read(
                *(struct FAT32DriverRequest*) frame.cpu.general.ebx
            );
            break;
        case 1:
            /*read directory*/
            *((int8_t*) frame.cpu.general.ecx) = read_directory(
                *(struct FAT32DriverRequest*) frame.cpu.general.ebx
            );
            break;
        case 2:
            /*write*/
            *((int8_t*) frame.cpu.general.ecx) = write(
                *(struct FAT32DriverRequest*) frame.cpu.general.ebx
            );
            break;
        case 3:
            /*delete*/
            *((int8_t*) frame.cpu.general.ecx) = delete(
                *(struct FAT32DriverRequest*) frame.cpu.general.ebx
            );
            break;
        case 4:
            /*get keyboard interup*/

            get_keyboard_buffer((char*) frame.cpu.general.ebx);
            break;
        case 5:
            /*put char*/
            if ((frame.cpu.general.ebx >= 32 && frame.cpu.general.ebx <= 127) || frame.cpu.general.ebx == 0x0){
                putchar(frame.cpu.general.ebx,4); // Assuming putc() exist in kernel
            }
            break;
        case 6:
            /* puts*/
            puts(
                (char*) frame.cpu.general.ebx, 
                (uint32_t) frame.cpu.general.ecx, 
                frame.cpu.general.edx
            );
            break;
        case 7: 
            /* activate keyboard interupt*/
            keyboard_state_activate();
            break;
        case 8:
            /* clear screen */
            framebuffer_clear();
            break;
        case 9:
            process_destroy(frame.cpu.general.ebx);
            puts((char*) "Process destroyed", 17, 0xF);
            puts((char*) "\n", 1, 0);
            break;
        case 10:
            struct ProcessControlBlock* pcb = process_get_current_running_pcb_pointer();

            puts((char*) "Process Name  : ", 16, 0xF);
            puts((char*) pcb->metadata.name, 8, 0xF);
            puts((char*) "\nProcess Id    : ", 17, 0xF);
            puts((char*) numbers[pcb->metadata.pid], 2, 0xF);
            puts((char*) "\nProcess State : ", 17, 0xF);
            if (pcb->metadata.state == 0) {
                puts((char*) "NO_PROCESS", 10, 0xF);
            } else if (pcb->metadata.state == 1) {
                puts((char*) "PROCESS_STATE_READY", 19, 0xF);
            } else if (pcb->metadata.state == 2) {
                puts((char*) "PROCESS_STATE_RUNNING", 21, 0xF);
            } else if (pcb->metadata.state == 3) {
                puts((char*) "PROCESS_STATE_BLOCKED", 21, 0xF);
            }
            puts((char*) "\n", 1, 0);
            break;
        case 11:
            // clock
            framebuffer_write(12, 40, 'A', 0xF, 0);

    }
}

void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
        case PIC1_OFFSET + IRQ_KEYBOARD:
            keyboard_isr();
            break;
        case 0x30:
            syscall(frame);
            break;
        case PIC1_OFFSET + IRQ_TIMER:
            struct Context ctx = {
                .cpu = frame.cpu,
                .eip = frame.int_stack.eip,
                .eflags = frame.int_stack.eflags,
                .page_directory_virtual_addr = paging_get_current_page_directory_addr(),
            };

            // framebuffer_write()

            scheduler_save_context_to_current_running_pcb(ctx);
            pic_ack(0);
            scheduler_switch_to_next_process();
            
            break;
    }
}

