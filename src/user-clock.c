// #include "header/cmos/cmos.h"
#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"


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

    while(true){
        syscall(11, 0, 0, 0);
    }


    return 0;
}

// #include <stdint.h>
// #include "header/filesystem/fat32.h"
// #include "header/stdlib/string.h"

// #define BLACK 0x00
// #define DARK_BLUE 0x01
// #define DARK_GREEN 0x2
// #define DARK_AQUA 0x3
// #define DARK_RED 0x4
// #define DARK_PURPLE 0x5
// #define GOLD 0x6
// #define GRAY 0x7
// #define DARK_GRAY 0x8
// #define BLUE 0x09
// #define GREEN 0x0A
// #define AQUA 0x0B
// #define RED 0x0C
// #define LIGHT_PURPLE 0x0D
// #define YELLOW 0x0E
// #define WHITE 0x0F

// #define KEYBOARD_BUFFER_SIZE 256

// int CURRENT_DIR_CLUSTER_NUMBER;
// int CURRENT_DIR_PARENT_CLUSTER_NUMBER;
// int DEPTH;
// char DIR_PATH[10][8];
// int DIR_CLUSTERS[10];
// char* numbers[17] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"}; 

// // int main(void) {
// //     __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(0xDEADBEEF));
// //     return 0;
// // }

// void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
//     __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
//     __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
//     __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
//     __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
//     // Note : gcc usually use %eax as intermediate register,
//     //        so it need to be the last one to mov
//     __asm__ volatile("int $0x30");
// }

// void print_terminal_text(){

//     syscall(6, (uint32_t) "JatOS-IF2230", 12, GREEN);
//     syscall(6, (uint32_t) ":", 1, GRAY);
//     // syscall(6, (uint32_t) curent_path, 1, BLUE);
//     int currDepth = DEPTH;
//     if (currDepth == 0){
//         syscall(6, (uint32_t) "/", 1, BLUE);
//     } else{
//         for (int i = 1; i <= currDepth; i++){
//             int dirLen = 0;
//             while (!(memcmp(&DIR_PATH[i][dirLen], "\0", 1) == 0) && (dirLen != 8)){
//                 dirLen++;
//             }
//             syscall(6, (uint32_t) "/", 1, BLUE);
//             syscall(6, (uint32_t) DIR_PATH[i], dirLen, BLUE);
//         }
//     }
//     syscall(6, (uint32_t) "$ ", 2, GRAY);
// }

// void ls(){

//     struct FAT32DirectoryTable dir_table;

//     struct FAT32DriverRequest request = {
//         .buf = &dir_table,
//         .ext = "",
//         .parent_cluster_number = CURRENT_DIR_PARENT_CLUSTER_NUMBER,
//         .buffer_size = sizeof(struct FAT32DirectoryTable),
//     };

//     int currDepth = DEPTH;

//     for (int i = 0; i < 8; i++){
//         request.name[i] = DIR_PATH[currDepth][i];
//     }

//     int8_t retcode;
//     syscall(1, (uint32_t)&request, (uint32_t)&retcode, 0);

//     if (retcode != 0){
//         syscall(6, (uint32_t) "ls gagal\n", 9, WHITE);
//         return;
//     } 

//     for (int i = 0; i < (int) (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++){
        
//         struct FAT32DirectoryEntry entry = dir_table.table[i];
//         if (entry.name[0] == 0){
//             continue;
//         }
//         int name_length = 0;
//         for (int j = 0; j < 8; j++){
//             if (entry.name[j] == '\0'){
//                 break;
//             }
//             name_length++;
//         }
//         if (i == 0){
//             syscall(6, (uint32_t) entry.name, name_length, DARK_GREEN);
//             syscall(6, (uint32_t) "/", 1, DARK_GREEN);
//         }
//         else if (entry.attribute == ATR_DIRECTORY){
//             syscall(6, (uint32_t) "  ", 2, WHITE);
//             syscall(6, (uint32_t) entry.name, name_length, WHITE);
//             syscall(6, (uint32_t) "/", 1, WHITE);
//         } else {
//             syscall(6, (uint32_t) "  ", 2, WHITE);
//             syscall(6, (uint32_t) entry.name, name_length, WHITE);
//             if (entry.ext[0] != '\0'){
//                 syscall(6, (uint32_t) ".", 1, WHITE);
//                 syscall(6, (uint32_t) entry.ext, 3, WHITE);
//             }
//         }
//     }
//     syscall(6, (uint32_t) "\n", 1, WHITE);
// }

// void cd(char* command, uint32_t length){
//     int currDepth = DEPTH;
//     uint32_t clusterNumber;
//     // uint32_t parentClusterNumber = CURRENT_DIR_PARENT_CLUSTER_NUMBER;

//     if (memcmp(command, "cd ..", 5) == 0){
//         if (currDepth > 0){
//             DIR_CLUSTERS[currDepth] = -1;
//             if (currDepth >= 2){
//                 CURRENT_DIR_PARENT_CLUSTER_NUMBER = DIR_CLUSTERS[currDepth - 2];
//             } else {
//                 CURRENT_DIR_PARENT_CLUSTER_NUMBER = DIR_CLUSTERS[currDepth - 1];
//             }
//             CURRENT_DIR_CLUSTER_NUMBER = DIR_CLUSTERS[currDepth - 1];
//             DEPTH--;
//         }
//         return;
//     } else {
//         uint32_t i = 3;
//         char path[8];
//         int pathLen = 0;
        
//         while (i < length){
//             pathLen = 0;
//             while ((command[i] != '/') && (i < length)){
//                 path[pathLen] = command[i];
//                 pathLen++;
//                 i++;
//             }

//             struct FAT32DirectoryTable dir_table;
//             // int clustNumber = -1;

//             struct FAT32DriverRequest request = {
//                 .buf = &dir_table,
//                 .ext = "",
//                 .buffer_size = sizeof(struct FAT32DirectoryTable),
//             };

//             if (currDepth == 0){
//                 request.parent_cluster_number = DIR_CLUSTERS[0];
//             } else {
//                 request.parent_cluster_number = DIR_CLUSTERS[currDepth - 1];
//             }

//             for (int j = 0; j < 8; j++){
//                 request.name[j] = DIR_PATH[currDepth][j];
//             }

//             int8_t retcode;
//             syscall(1, (uint32_t)&request, (uint32_t)&retcode, 0);

//             if (retcode != 0){
//                 syscall(6, (uint32_t) "failed to read request\n", 23, RED);
//                 syscall(6, (uint32_t) "directory tidak ditemukan\n", 26, RED);
//                 return;
//             }

//             bool found = false;
//             for (int j = 1; j < (int) (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); j++){
//                 struct FAT32DirectoryEntry entry = dir_table.table[j];
//                 if (entry.name[0] == 0){
//                     continue;
//                 }

//                 clusterNumber = entry.cluster_low | (entry.cluster_high << 16);
//                 int nameLength = 0;

//                 for (int k = 0; k < 8; k++){
//                     if (entry.name[k] == '\0'){
//                         break;
//                     }
//                     nameLength++;
//                 }

//                 if ((entry.attribute == ATR_DIRECTORY) && (pathLen == nameLength)){
//                     if (memcmp(entry.name, path, pathLen) == 0){
//                         // memcpy(DIR_PATH[currDepth], entry.name, 8);
//                         currDepth++;
//                         for (int k = 0; k < 8; k++){
//                             DIR_PATH[currDepth][k] = entry.name[k];
//                         }
//                         found = true;
//                         DIR_CLUSTERS[currDepth] = clusterNumber;
//                         // if (i != length){
//                         //     parentClusterNumber = clusterNumber;
//                         // }
//                         break;
//                     }
//                 }
//             }

//             if (!found){
//                 syscall(6, (uint32_t) "directory tidak ditemukan\n", 26, RED);
//                 return;
//             }
//             i++;
//         }
//         DEPTH = currDepth;
//         CURRENT_DIR_PARENT_CLUSTER_NUMBER = DIR_CLUSTERS[currDepth-1];
//         CURRENT_DIR_CLUSTER_NUMBER = DIR_CLUSTERS[currDepth];
//     }
// }

// void cat(char* filename){
    

//     // syscall(6, (uint32_t) "meoww ", 6, WHITE);
//     // syscall(6, (uint32_t) filename, 8, WHITE);
//     // syscall(6, (uint32_t) "\n", 1, WHITE);

//     struct FAT32DirectoryTable dir_table;
//     struct FAT32DriverRequest request = {
//         .buf = &dir_table,
//         .ext = "",
//         .parent_cluster_number = CURRENT_DIR_PARENT_CLUSTER_NUMBER,
//         .buffer_size = sizeof(struct FAT32DirectoryTable),
//     };

//     int currDepth = DEPTH;

//     for (int i = 0; i < 8; i++){
//         request.name[i] = DIR_PATH[currDepth][i];
//     }

//     int8_t retcode;
//     syscall(1, (uint32_t)&request, (uint32_t)&retcode, 0);

//     if (retcode != 0){
//         syscall(6, (uint32_t) "dir table corrupt\n", 17, WHITE);
//         return;
//     } 


//     uint32_t clust_number = 0;
//     // struct FAT32DirectoryEntry read_entry;

//     int file_size = 0;

//     for (int i = 0; i < (int) (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++){
//         struct FAT32DirectoryEntry entry = dir_table.table[i];
//         if (memcmp(entry.name, filename, 8) == 0){
//             clust_number = entry.cluster_low | (entry.cluster_high << 16);
//             file_size = entry.filesize;
//             break;
//         }
//     }

//     if (clust_number == 0){
//         syscall(6, (uint32_t) "file tidak tersedia\n", 20, WHITE);
//         return;
//     } 

//     // syscall(6, (uint32_t) "File found at cluster ", 22, WHITE);
//     // syscall(6, (uint32_t) clust_number, 2, WHITE); <-- buffer overflow, ngerusak return addr

//     // uint8_t read_buffer[292];
//     int req_size = CLUSTER_SIZE;
//     while (req_size < file_size){
//         req_size += CLUSTER_SIZE;
//     }
//     uint8_t read_buffer[req_size];

//     struct FAT32DriverRequest request_read = {
//         .buf = &read_buffer,
//         .parent_cluster_number = CURRENT_DIR_PARENT_CLUSTER_NUMBER,
//         .buffer_size = req_size,
//     };

//     for (int i = 0; i < 8; i++) {
//         request_read.name[i] = filename[i];
//     }


//     syscall(0, (uint32_t)&request_read, (uint32_t)&retcode, 0);

//     if (retcode != 0){
//         syscall(6, (uint32_t) "bukan file\n", 11, WHITE);
//         return;
//     }

//     syscall(6, (uint32_t) read_buffer, (uint32_t) file_size, GOLD);
//     syscall(6, (uint32_t) "\n", 1, WHITE);
// }

// void mkdir(char* foldername){

//     // syscall(6, (uint32_t) "mkdir\n", 6, WHITE);
//     // syscall(6, (uint32_t) foldername, 8, WHITE);
//     struct FAT32DirectoryTable dir_table;
//     struct FAT32DriverRequest request = {
//         .buf = &dir_table,
//         .ext = "",
//         .parent_cluster_number = CURRENT_DIR_PARENT_CLUSTER_NUMBER,
//         .buffer_size = sizeof(struct FAT32DirectoryTable),
//     };

//     int currDepth = DEPTH;

//     for (int i = 0; i < 8; i++){
//         request.name[i] = DIR_PATH[currDepth][i];
//     }

//     int8_t retcode;
//     syscall(1, (uint32_t)&request, (uint32_t)&retcode, 0);

//     if (retcode != 0){
//         syscall(6, (uint32_t) "dir table corrupt\n", 17, WHITE);
//         return;
//     } 

//     struct FAT32DriverRequest request_write = {
//         .parent_cluster_number = CURRENT_DIR_CLUSTER_NUMBER,
//         .buffer_size = 0,
//         .ext = "\0\0\0",
//     };

//     for (int i = 0; i < 8; i++) {
//         request_write.name[i] = foldername[i];
//     }

//     syscall(2, (uint32_t)&request_write, (uint32_t)&retcode, 0);


//     if (retcode != 0){
//         syscall(6, (uint32_t) "gagal\n", 6, WHITE);
//         return;
//     }

//     // syscall(6, (uint32_t) "sukses!", 7, WHITE);
//     // syscall(6, (uint32_t) "\n", 1, WHITE);
// }

// void rm(char* filename){
    
//     // syscall(6, (uint32_t) "meoww ", 6, WHITE);
//     // syscall(6, (uint32_t) filename, 8, WHITE);
//     // syscall(6, (uint32_t) "\n", 1, WHITE);

//     struct FAT32DirectoryTable dir_table;
//     struct FAT32DriverRequest request = {
//         .buf = &dir_table,
//         .ext = "",
//         .parent_cluster_number = CURRENT_DIR_PARENT_CLUSTER_NUMBER,
//         .buffer_size = sizeof(struct FAT32DirectoryTable),
//     };

//     int currDepth = DEPTH;

//     for (int i = 0; i < 8; i++){
//         request.name[i] = DIR_PATH[currDepth][i];
//     }

//     int8_t retcode;
//     syscall(1, (uint32_t)&request, (uint32_t)&retcode, 0);

//     if (retcode != 0){
//         syscall(6, (uint32_t) "dir table corrupt\n", 17, WHITE);
//         return;
//     } 


//     uint32_t clust_number = 0;
//     // struct FAT32DirectoryEntry read_entry;

//     for (int i = 0; i < (int) (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++){
//         struct FAT32DirectoryEntry entry = dir_table.table[i];
//         if (memcmp(entry.name, filename, 8) == 0){
//             clust_number = entry.cluster_low | (entry.cluster_high << 16);
//             break;
//         }
//     }

//     if (clust_number == 0){
//         syscall(6, (uint32_t) "file tidak tersedia\n", 20, WHITE);
//         return;
//     } 

//     // syscall(6, (uint32_t) "File found at cluster ", 22, WHITE);
//     // syscall(6, (uint32_t) clust_number, 2, WHITE); <-- buffer overflow, ngerusak return addr

//     // uint8_t read_buffer[292];
//     // int req_size = CLUSTER_SIZE;
//     // while (req_size < file_size){
//     //     req_size += CLUSTER_SIZE;
//     // }
//     // uint8_t read_buffer[req_size];

//     struct FAT32DriverRequest request_delete = {
//         .parent_cluster_number = CURRENT_DIR_PARENT_CLUSTER_NUMBER,
//         .ext = "txt",
//     };

//     for (int i = 0; i < 8; i++) {
//         request_delete.name[i] = filename[i];
//     }


//     syscall(3, (uint32_t)&request_delete, (uint32_t)&retcode, 0);

//     if (retcode != 0){
//         syscall(6, (uint32_t) "bukan file\n", 11, WHITE);
//         return;
//     }

//     // syscall(6, (uint32_t) "sukses!", 7, WHITE);
//     // syscall(6, (uint32_t) "\n", 1, WHITE);
// }

// void kill(char* pid){
//     bool found = false;
//     int pidInt = -1;
//     for (int i = 0; i < 17; i++){
//         if (memcmp(numbers[i], pid, 2) == 0){
//             found = true;
//             pidInt = i;
//             break;
//         }
//     }
//     // syscall(6, (uint32_t) "kill detected: ", 15, WHITE);

//     if (!found){
//         syscall(6, (uint32_t) "pid tidak valid\n", 16, RED);
//         return;
//     }

//     // syscall(6, (uint32_t) pid, 2, WHITE);
//     // syscall(6, (uint32_t) "\n", 1, WHITE);
//     syscall(9, pidInt, 0, 0);
// }

// void executeCommand(char* command, uint32_t length){
//     char* CD = "cd ";
//     char* LS = "ls ";
//     char* MKDIR = "mkdir ";
//     char* CAT = "cat ";
//     char* CP = "cp ";
//     char* RM = "rm ";
//     char* MV = "mv ";
//     char* FIND = "find ";
//     char* CLEAR = "clear";
//     char* EXEC = "exec ";
//     char* PS = "ps";
//     char* KILL = "kill";
//     // char buf[11] = "cd detected";
//     // syscall(6, (uint32_t) "masuk", 5, WHITE);
//     // syscall(6, (uint32_t) "\n", 1, WHITE);

//     if (memcmp(command, LS, 2) == 0){
//             ls();
//         }
//     else if (memcmp(command, PS, 2) == 0){
//         syscall(10, 0, 0, 0);
//     }

//     if (length > 3){
//         if (memcmp(command, CD, 3) == 0){
//             cd(command, length);

//         } else if (memcmp(command, CP, 3) == 0){
//             syscall(6, (uint32_t) "cp detected", 11, WHITE);
//             syscall(6, (uint32_t) "\n", 1, WHITE);

//         } else if (memcmp(command, RM, 3) == 0){

//             if (length > 11){
//                 syscall(6, (uint32_t) "nama file terlalu panjang\n", 26, WHITE);
//                 return;
//             }
            
//             char filename[length-3];
//             memcpy(filename, command + 3, length-3);
//             memset(filename + length-3, '\0', 11-length);

//             rm(filename);

//             return;

//         } else if (memcmp(command, MV, 3) == 0){
//             syscall(6, (uint32_t) "mv detected", 11, WHITE);
//             syscall(6, (uint32_t) "\n", 1, WHITE);
//         } 
//         if (length > 4){
//             if (memcmp(command, CAT, 4) == 0){

//                 if (length > 12){
//                     syscall(6, (uint32_t) "nama file terlalu panjang\n", 26, WHITE);
//                     return;
//                 }

//                 char filename[length-4];
//                 memcpy(filename, command + 4, length-4);
//                 memset(filename + length-4, '\0', 12-length);

//                 cat(filename);

//                 return;
//             }
//             else if (memcmp(command, CLEAR, 5) == 0){
//                 syscall(8, 0, 0, 0);

//                 return;
//             }
//             else if (memcmp(command, EXEC, 5) == 0){

//             }
//         }

//         if (length > 5){
//             if (memcmp(command, FIND, 5) == 0){
//                 syscall(6, (uint32_t) "find detected", 13, WHITE);
//                 syscall(6, (uint32_t) "\n", 1, WHITE);

//                 return;
//             } else if (memcmp(command, KILL, 4) == 0){

//                 char pid[2] = {'\0', '\0'};
//                 uint32_t i = 5;
//                 while (i < length){
//                     pid[i-5] = command[i];
//                     i++;
//                 }

//                 if (length == 5){
//                     pid[1] = '\0';
//                 }

//                 kill(pid);
//             }

//         }

//         if (length > 6){
//             if (memcmp(command, MKDIR, 6) == 0){

//                 if (length > 14){
//                     syscall(6, (uint32_t) "nama folder terlalu panjang\n", 28, WHITE);
//                     return;
//                 }

//                 char foldername[length-6];
//                 memcpy(foldername, command + 6, length-6);
//                 memset(foldername + length-6, '\0', 14-length);

//                 mkdir(foldername);
                
//                 return;
//             }
//         }
//     }
// }

// int main(void) {

//     syscall(7, 0, 0, 0);

//     memcpy(DIR_PATH[0], "root\0\0\0\0", 8);
//     for (int i = 1; i < 10; i++){
//         memcpy(DIR_PATH[i], "\0\0\0\0\0\0\0\0", 8);
//     }
//     DEPTH = 0;
//     CURRENT_DIR_CLUSTER_NUMBER = 2;
//     CURRENT_DIR_PARENT_CLUSTER_NUMBER = 2;
//     for (int i = 0; i < 10; i++){
//         DIR_CLUSTERS[i] = -1;
//     }
//     DIR_CLUSTERS[0] = 2;
//     // memcpy(DIR_PATH[1], "test\0\0\0\0", 8);
//     // DEPTH++;
//     print_terminal_text();

//     char buf[KEYBOARD_BUFFER_SIZE];
//     for (int i = 0; i < KEYBOARD_BUFFER_SIZE; i++) {
//         buf[i] = 0;
//     }

//     int i = 0;
//     char keyboard_input = 0;

//     syscall(8, 0, 0, 0);
//     print_terminal_text();
//     while (true) {
//         syscall(4, (uint32_t) &keyboard_input, 0, 0);

//         if (keyboard_input == 0){
//             continue;
//         } else if (keyboard_input == '\b'){
//             if (i > 0){
//                 i--;
//                 buf[i] = 0;
//             } else {
//                 syscall(6, (uint32_t) "", 0, 0);
//             }
//         }
//         else if (keyboard_input == '\n') {
//             syscall(6, (uint32_t) "\n", 1, WHITE);

//             executeCommand(buf, i);
//             i = 0;

//             for (int j = 0; j < KEYBOARD_BUFFER_SIZE; j++) {
//                 buf[j] = 0;
//             }

//             print_terminal_text();
//         } else {
//             buf[i] = keyboard_input;
//             i++;
//         }
//     }

//     return 0;
// }