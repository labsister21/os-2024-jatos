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
#include "header/driver/disk.h"
#include "header/filesystem/fat32.h"
#include "header/memory/paging.h"
#include "header/process/process.h"

// USER MODE TEST
void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_write(0, 0, 0, 0xF, 0);
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    char text2[264] = "Hololive Production is a virtual YouTuber agency owned by Japanese tech entertainment company Cover Corporation. In addition to acting as a multi-channel network, Hololive Production also handles licensing, merchandising, music production and concert organization.";
    struct FAT32DriverRequest request2 = {
        .buf                   = text2,
        .name                  = "hololive",
        .ext                   = "txt",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 266,
    };
    write(request2);

    char text3[218] = "A VTuber or virtual YouTuber is an online entertainer who uses a virtual avatar generated using computer graphics. Real-time motion capture software or technology are often—but not always—used to capture movement.";
    struct FAT32DriverRequest request3 = {
        .buf                   = text3,
        .name                  = "vtuber\0\0",
        .ext                   = "txt",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 218,
    };
    write(request3);

    char text4[292] = "Hoshimachi Suisei is a Japanese virtual YouTuber. She began posting videos as an independent creator in March 2018. In May 2019, she became affiliated with Hololive Production through their newly created music label, INoNaKa Music, before joining the agency's main branch later the same year.";
     struct FAT32DriverRequest request4 = {
        .buf                   = text4,
        .name                  = "suisei\0\0",
        .ext                   = "txt",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 292,
    };
    write(request4);

    struct FAT32DriverRequest request5 = {
        .name                  = "fotozeta",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0,
    };
    write(request5);

    char text6[208] =  "Vestia Zeta adalah YouTuber Virtual wanita Indonesia yang terkait dengan hololive, memulai debutnya sebagai bagian dari cabang VTubers generasi ketiga Indonesia (ID) bersama Kaela Kovalskia dan Kobo Kanaeru.";

    struct FAT32DriverRequest request6 = {
        .buf                   = text6,
        .name                  = "zeta\0\0\0\0",
        .ext                   = "txt",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 208,
    };
    write(request6);



      // Allocate first 4 MiB virtual memory
    paging_allocate_user_page_frame(&_paging_kernel_page_directory, (uint8_t*) 0);

    // Write shell into memory
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x100000,
    };
    // read(request);

    set_tss_kernel_current_stack();

    process_create_user_process(request);
    paging_use_page_directory(_process_list[0].context.page_directory_virtual_addr);
    // Set TSS $esp pointer and jump into shell 
    kernel_execute_user_program((void*) 0x0);
}
