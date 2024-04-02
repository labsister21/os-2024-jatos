#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

struct FAT32DriverState driver_state;

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

uint32_t cluster_to_lba(uint32_t cluster){
    
}


/**
 * Initialize DirectoryTable value with 
 * - Entry-0: DirectoryEntry about itself
 * - Entry-1: Parent DirectoryEntry
 * 
 * @param dir_table          Pointer to directory table
 * @param name               8-byte char for directory name
 * @param parent_dir_cluster Parent directory cluster number
 */
void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    driver_state.dir_table_buf.table[0].user_attribute = UATTR_NOT_EMPTY;
    driver_state.dir_table_buf.table[1].user_attribute = UATTR_NOT_EMPTY;
    driver_state.dir_table_buf.table[1].attribute = ATTR_SUBDIRECTORY;

    for(int i = 0 ; i < 8 ; i++){
        driver_state.dir_table_buf.table[0].name[i] = name[i];
    }

    for(int i = 0 ; i < 3;i++){
        driver_state.dir_table_buf.table[0].ext[i] = '\0';
    }

    for(int i = 0 ;i < 8; i++){
        driver_state.dir_table_buf.table[1].name[i] = '\0';
    }

    for(int i = 0 ; i < 3;i++){
        driver_state.dir_table_buf.table[1].ext[i] = '\0';
    }

    driver_state.dir_table_buf.table[1].cluster_low = parent_dir_cluster & 0xFFFF;
    driver_state.dir_table_buf.table[1].cluster_high = parent_dir_cluster >> 16;    
}











