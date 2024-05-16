#include <stdint.h>
// #include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"
#include "header/cmos/cmos.h"


void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

int main(void) {
    
    syscall(11, 0, 0, 0);

    char time_array[6]; 

    while(true){
   
        syscall(12, (uint32_t) time_array, 0, 0);
        syscall(13, 24, 72, (uint32_t) time_array[0]);
        syscall(13, 24, 73, (uint32_t) time_array[1]);
        syscall(13, 24, 74, ':');
        syscall(13, 24, 75, (uint32_t) time_array[2]);
        syscall(13, 24, 76, (uint32_t) time_array[3]);
        syscall(13, 24, 77, ':');
        syscall(13, 24, 78, (uint32_t) time_array[4]);
        syscall(13, 24, 79, (uint32_t) time_array[5]);
    }


    return 0;
}
