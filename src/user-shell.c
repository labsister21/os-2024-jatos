#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"

#define BLACK 0x00
#define DARK_BLUE 0x01
#define DARK_GREEN 0x2
#define DARK_AQUA 0x3
#define DARK_RED 0x4
#define DARK_PURPLE 0x5
#define GOLD 0x6
#define GRAY 0x7
#define DARK_GRAY 0x8
#define BLUE 0x09
#define GREEN 0x0A
#define AQUA 0x0B
#define RED 0x0C
#define LIGHT_PURPLE 0x0D
#define YELLOW 0x0E
#define WHITE 0x0F
#define KEYBOARD_BUFFER_SIZE 256

// int main(void) {
//     __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(0xDEADBEEF));
//     return 0;
// }

void executeCommand(char* command, uint32_t length);

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void print_terminal_text(char* curent_path){

    syscall(6, (uint32_t) "JatOS-IF2230", 12, GREEN);
    syscall(6, (uint32_t) ":", 1, GRAY);
    syscall(6, (uint32_t) curent_path, 1, BLUE);
    syscall(6, (uint32_t) "$ ", 2, GRAY);
}

int strcmp(char* s1, char* s2) {
  int i = 0;
  while (s1[i] == s2[i]) {
    if (s1[i] == '\0') {
      return 0;
    }
    i++;
  }
  return s1[i] - s2[i];
}

int main(void) {

    syscall(7, 0, 0, 0);

    char* current_path = "/";
    print_terminal_text(current_path);

    char buf[KEYBOARD_BUFFER_SIZE];
    for (int i = 0; i < KEYBOARD_BUFFER_SIZE; i++) {
        buf[i] = 0;
    }

    int i = 0;
    char keyboard_input = 0;

    while (true) {
        syscall(4, (uint32_t) &keyboard_input, 0, 0);

        if (keyboard_input == 0){
            continue;
        } else if (keyboard_input == '\b'){
            if (i > 0){
                i--;
                buf[i] = 0;
            } else {
                syscall(6, (uint32_t) "", 0, 0);
            }
        }
        else if (keyboard_input == '\n') {
            syscall(6, (uint32_t) "\n", 1, WHITE);

            executeCommand(buf, i);
            i = 0;

            for (int j = 0; j < KEYBOARD_BUFFER_SIZE; j++) {
                buf[j] = 0;
            }

            print_terminal_text(current_path);
        } else {
            buf[i] = keyboard_input;
            i++;
        }
    }

    return 0;
}

void executeCommand(char* command, uint32_t length){
    char* CD = "cd ";
    char* LS = "ls ";
    char* MKDIR = "mkdir ";
    char* CAT = "cat ";
    char* CP = "cp ";
    char* RM = "rm ";
    char* MV = "mv ";
    char* FIND = "find ";
    char* CLEAR = "clear";

    // char buf[11] = "cd detected";
    // syscall(6, (uint32_t) "masuk", 5, WHITE);
    // syscall(6, (uint32_t) "\n", 1, WHITE);

    if (memcmp(command, LS, 2) == 0){
            syscall(6, (uint32_t) "ls detected", 11, WHITE);
            syscall(6, (uint32_t) "\n", 1, WHITE);
        }

    if (length > 3){
        if (memcmp(command, CD, 3) == 0){
            syscall(6, (uint32_t) "cd detected", 11, WHITE);
            syscall(6, (uint32_t) "\n", 1, WHITE);
        } else if (memcmp(command, CP, 3) == 0){
            syscall(6, (uint32_t) "cp detected", 11, WHITE);
            syscall(6, (uint32_t) "\n", 1, WHITE);
        } else if (memcmp(command, RM, 3) == 0){
            syscall(6, (uint32_t) "rm detected", 11, WHITE);
            syscall(6, (uint32_t) "\n", 1, WHITE);
        } else if (memcmp(command, MV, 3) == 0){
            syscall(6, (uint32_t) "mv detected", 11, WHITE);
            syscall(6, (uint32_t) "\n", 1, WHITE);
        }

        if (length > 4){
            if (memcmp(command, CAT, 4) == 0){
                syscall(6, (uint32_t) "cat detected", 12, WHITE);
                syscall(6, (uint32_t) "\n", 1, WHITE);
            }
            else if (memcmp(command, CLEAR, 5) == 0){
                syscall(8, 0, 0, 0);
            
            }
        }

        if (length > 5){
            if (memcmp(command, FIND, 5) == 0){
                syscall(6, (uint32_t) "find detected", 13, WHITE);
                syscall(6, (uint32_t) "\n", 1, WHITE);
            }
        }

        if (length > 6){
            if (memcmp(command, MKDIR, 6) == 0){
                syscall(6, (uint32_t) "mkdir detected", 14, WHITE);
                syscall(6, (uint32_t) "\n", 1, WHITE);
            }
        }
    }
}