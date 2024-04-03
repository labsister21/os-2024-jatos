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

/* -- Driver Interfaces -- */

/**
 * Convert cluster number to logical block address
 * 
 * @param cluster Cluster number to convert
 * @return uint32_t Logical Block Address
 */
uint32_t cluster_to_lba(uint32_t cluster){
    return cluster * CLUSTER_MAP_SIZE;
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

/**
 * Checking whether filesystem signature is missing or not in boot sector
 * 
 * @return True if memcmp(boot_sector, fs_signature) returning inequality
 */
bool is_empty_storage(void){
    uint8_t boot_sector[BLOCK_SIZE];
    read_blocks(boot_sector, 0, 1);
    return memcmp(boot_sector, fs_signature, BLOCK_SIZE);
}

/**
 * Create new FAT32 file system. Will write fs_signature into boot sector and 
 * proper FileAllocationTable (contain CLUSTER_0_VALUE, CLUSTER_1_VALUE, 
 * and initialized root directory) into cluster number 1
 */
void create_fat32(void){
    // inisiasi boot sector (ga boleh disentuh, reset)
    uint8_t boot_sector[BLOCK_SIZE];
    memset(boot_sector,0,BLOCK_SIZE);
    memcpy(boot_sector,fs_signature,BLOCK_SIZE);
    write_blocks(boot_sector, 0, 1);

    // sector pertama
    driver_state.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driver_state.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    driver_state.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;
    write_clusters(&driver_state.fat_table.cluster_map, 1, 1);

    // sector kedua isinya root
    struct FAT32DirectoryTable directory_table = {0};
    init_directory_table(&directory_table,"root\0\0\0\0",2);
    write_clusters(&directory_table, 2, 1);

    for(int i = 3 ; i < CLUSTER_SIZE; i++){
        driver_state.fat_table.cluster_map[i] = FAT32_FAT_EMPTY_ENTRY;
    }
}

/**
 * Initialize file system driver state, if is_empty_storage() then create_fat32()
 * Else, read and cache entire FileAllocationTable (located at cluster number 1) into driver state
 */
void initialize_filesystem_fat32(void){
    if (is_empty_storage()){
        create_fat32();
    } else {
        read_clusters(&driver_state.fat_table.cluster_map, 1, 1);
    }
}

/**
 * Write cluster operation, wrapper for write_blocks().
 * Recommended to use struct ClusterBuffer
 * 
 * @param ptr            Pointer to source data
 * @param cluster_number Cluster number to write
 * @param cluster_count  Cluster count to write, due limitation of write_blocks block_count 255 => max cluster_count = 63
 */
void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    struct ClusterBuffer* cluster_buffer = ptr;
    write_blocks(cluster_buffer, cluster_to_lba(cluster_number), cluster_count*CLUSTER_BLOCK_COUNT);
}

/**
 * Read cluster operation, wrapper for read_blocks().
 * Recommended to use struct ClusterBuffer
 * 
 * @param ptr            Pointer to buffer for reading
 * @param cluster_number Cluster number to read
 * @param cluster_count  Cluster count to read, due limitation of read_blocks block_count 255 => max cluster_count = 63
 */
void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    struct ClusterBuffer* cluster_buffer = ptr;
    read_blocks(cluster_buffer, cluster_to_lba(cluster_number), cluster_count*CLUSTER_BLOCK_COUNT);
}



/* -- CRUD Operation -- */

/**
 *  FAT32 Folder / Directory read
 *
 * @param request buf point to struct FAT32DirectoryTable,
 *                name is directory name,
 *                ext is unused,
 *                parent_cluster_number is target directory table to read,
 *                buffer_size must be exactly sizeof(struct FAT32DirectoryTable)
 * @return Error code: 0 success - 1 not a folder - 2 not found - -1 unknown
 */
int8_t read_directory(struct FAT32DriverRequest request){
    if (request.buffer_size != sizeof(driver_state.dir_table_buf)){
        return -1;
    }

    // loop cari idx nya
    struct FAT32DirectoryTable *dir_table;
    read_clusters(&dir_table, request.parent_cluster_number, 1);

    for (int i = 0; i < (CLUSTER_SIZE / 32); i++){
        if (dir_table->table[i].name == request.name){
            if (dir_table->table[i].ext != '\0'){
                return 1;
            } else {
                uint32_t clust_number = dir_table->table[i].cluster_low | (dir_table->table[i].cluster_high << 16);
                read_clusters(request.buf, clust_number, 1);
                return 0;
            }
        }
    }

    return 2;
}


/**
 * FAT32 read, read a file from file system.
 *
 * @param request All attribute will be used for read, buffer_size will limit reading count
 * @return Error code: 0 success - 1 not a file - 2 not enough buffer - 3 not found - -1 unknown
 */
int8_t read(struct FAT32DriverRequest request){
    if (request.buffer_size > sizeof(driver_state.dir_table_buf)){
        return 2;
    }

    struct FAT32DirectoryTable *dir_table;
    read_clusters(&dir_table, request.parent_cluster_number, 1);

    for (int i = 0; i < (CLUSTER_SIZE / 32); i++){
        if (dir_table->table[i].name == request.name){
            if (dir_table->table[i].filesize == 0){
                return 1;
            }
            else{
                uint32_t clust_number = dir_table->table[i].cluster_low | (dir_table->table[i].cluster_high << 16);
                while(driver_state.fat_table.cluster_map[clust_number] != FAT32_FAT_END_OF_FILE){
                    read_clusters(request.buf,clust_number,1);
                    clust_number = driver_state.fat_table.cluster_map[clust_number];
                    request.buf += CLUSTER_SIZE;
                }

            }
        }
    }


    return 3;

}

/**
 * FAT32 write, write a file or folder to file system.
 *
 * @param request All attribute will be used for write, buffer_size == 0 then create a folder / directory
 * @return Error code: 0 success - 1 file/folder already exist - 2 invalid parent cluster - -1 unknown
 */
int8_t write(struct FAT32DriverRequest request){
 
    uint32_t reqSize = request.buffer_size;
    uint32_t clusterNeed = reqSize / CLUSTER_SIZE;
    uint32_t clustBefore = 0;
    
    for (int i = 3 ; i < (CLUSTER_SIZE); i++){
        if(memcmp(request.name ,driver_state.dir_table_buf.table[i].name,8 ) == 0){
            return 1;
        }
    }
    

    if(driver_state.fat_table.cluster_map[request.parent_cluster_number] == FAT32_FAT_EMPTY_ENTRY){
        return 2;
    }
    

    if(clusterNeed * CLUSTER_SIZE < reqSize){
        clusterNeed++;
    }

    //WRITE FOLDER
    if(request.buffer_size == 0){
        for(int i = 3 ; i < CLUSTER_SIZE; i++){
            if(driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY ){
                write_clusters(request.buf,i,1);
                driver_state.fat_table.cluster_map[i] = FAT32_FAT_END_OF_FILE;
            }
        }
    }


    else{
        //WRITE FILE
        uint32_t written = 0;
        while(written < clusterNeed){
            for (int i = 3 ; i < (CLUSTER_SIZE); i++){

                if(driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY ){
                    write_clusters(request.buf+written * CLUSTER_SIZE,i,1);
                    written++;

                    if(written == 0 ){
                        //DO Nothing
                    }else {
                        driver_state.fat_table.cluster_map[clustBefore] = i;
                    }
                    if (written == clusterNeed){
                        driver_state.fat_table.cluster_map[i] = FAT32_FAT_END_OF_FILE;
                    }
                    clustBefore = i;
                }
            }
        }
    }  
    return 0;
}


/**
 * FAT32 delete, delete a file or empty directory (only 1 DirectoryEntry) in file system.
 *
 * @param request buf and buffer_size is unused
 * @return Error code: 0 success - 1 not found - 2 folder is not empty - -1 unknown
 */
int8_t delete(struct FAT32DriverRequest request){

}