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
    return cluster * CLUSTER_BLOCK_COUNT;
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

    struct FAT32DirectoryEntry *new_entry = &(dir_table->table[0]);

    memcpy(&(new_entry->name), name, 8);
    memcpy(&(new_entry->ext), "\0\0\0", 3);

    new_entry->attribute = ATTR_SUBDIRECTORY;
    new_entry->user_attribute = UATTR_NOT_EMPTY;

    new_entry->cluster_high = (uint16_t)(parent_dir_cluster >> 16);
    new_entry->cluster_low = (uint16_t)(parent_dir_cluster & 0xFFFF);
    new_entry->filesize = 0;
    
    dir_table->table[0] = *new_entry;
}



/**
 * Checking whether filesystem signature is missing or not in boot sector
 * 
 * @return True if memcmp(boot_sector, fs_signature) returning inequality
 */
bool is_empty_storage(void){
    uint8_t boot_sector[BLOCK_SIZE];
    read_blocks(boot_sector, 0, 1);
    return memcmp(boot_sector, fs_signature, BLOCK_SIZE) != 0;
}

/**
 * Create new FAT32 file system. Will write fs_signature into boot sector and 
 * proper FileAllocationTable (contain CLUSTER_0_VALUE, CLUSTER_1_VALUE, 
 * and initialized root directory) into cluster number 1
 */
void create_fat32(void){

    /* CLUSTER 0 */
    // menulis signature ke boot sector ke block 0 (cluster 0)
    write_blocks(fs_signature, BOOT_SECTOR, 1);

    /* CLUSTER 1 */
    // mengisi map value FAT table
    driver_state.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driver_state.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    driver_state.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;
    // for(int i = 3 ; i < CLUSTER_SIZE; i++){
    //     driver_state.fat_table.cluster_map[i] = FAT32_FAT_EMPTY_ENTRY;
    // }
    // menulis FAT table ke cluster 1 (FAT table cluster)
    write_clusters(&(driver_state.fat_table), FAT_CLUSTER_NUMBER, 1);

    /* CLUSTER 2 */
    // menginisiasi value dir_table_buf dengan root directory
    init_directory_table(&(driver_state.dir_table_buf), "root\0\0\0\0", ROOT_CLUSTER_NUMBER);

    // menulis dir_table_buf ke cluster 2 (root dir table cluster)
    write_clusters(&(driver_state.dir_table_buf), ROOT_CLUSTER_NUMBER, 1); 
}

/**
 * Initialize file system driver state, if is_empty_storage() then create_fat32()
 * Else, read and cache entire FileAllocationTable (located at cluster number 1) into driver state
 */
void initialize_filesystem_fat32(void){
    if (is_empty_storage()){
        create_fat32();
    } else {
        // menulis FAT table ke driverstate
        read_clusters(&(driver_state.fat_table), FAT_CLUSTER_NUMBER, 1);
        // menulis root directory ke driverstate
        read_clusters(&(driver_state.dir_table_buf), ROOT_CLUSTER_NUMBER, 1);
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
    // struct ClusterBuffer* cluster_buffer = ptr;
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count*CLUSTER_BLOCK_COUNT);
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

    // Ngecek apakah size request sesuai dengan ukuran FAT32DirectoryTable
    if (request.buffer_size != sizeof(driver_state.dir_table_buf)){
        return -1;
    }

    struct FAT32DirectoryTable dir_table;
    read_clusters(&dir_table, request.parent_cluster_number, 1);

    for (int i = 0; i < (CLUSTER_SIZE / 32); i++){
        if (memcmp(dir_table.table[i].name, request.name, 8) == 0){
            if (dir_table.table[i].ext[0] != '\0'){ 
                // request bukan folder
                return 1;
            } else {
                // request adalah folder
                uint32_t clust_number = dir_table.table[i].cluster_low | (dir_table.table[i].cluster_high << 16);
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

    // Jika nama file request kosong
    if (memcmp(request.name, "\0\0\0\0\0\0\0\0", 8) == 0){
        return -1;
    }



    // struct untuk parent dir
    struct FAT32DirectoryTable dir_table;
    uint32_t parent_cluster_number = request.parent_cluster_number;
    read_clusters(&dir_table, parent_cluster_number, 1);

    // looping untuk mencari request di parent dir table
    for (int i = 0; i < (CLUSTER_SIZE / 32); i++){

        // request ditemukan di parent dir table
        if (memcmp(dir_table.table[i].name, request.name, 8) == 0){

            // request adalah folder
            if (dir_table.table[i].filesize == 0){
                return 1;
            }

            // request adalah file
            else{

                // cari cluster number dari request
                uint32_t clust_number = dir_table.table[i].cluster_low | (dir_table.table[i].cluster_high << 16);
                uint32_t cluster_size = 0;

                // looping untuk membaca request dari disk ke request.buf
                do {
                    
                    // cek apakah request.buf masih cukup?
                    if (cluster_size*CLUSTER_MAP_SIZE > request.buffer_size){
                        return 2;
                    }
                    
                    read_clusters(request.buf,clust_number,1);
                    clust_number = driver_state.fat_table.cluster_map[clust_number];
                    
                    // Meng offset request.buf agar siap diisi lagi
                    request.buf += CLUSTER_SIZE;

                    cluster_size++;
                } while(clust_number != FAT32_FAT_END_OF_FILE);
                return 0;
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
 
    // parent cluster tidak valid
    if(driver_state.fat_table.cluster_map[request.parent_cluster_number] == FAT32_FAT_EMPTY_ENTRY){
        return 2;
    } 

    // menulis dir parent cluster ke drivestate.dir_table_buf
    read_clusters(&(driver_state.dir_table_buf), request.parent_cluster_number, 1);
    
    uint32_t reqSize = request.buffer_size;
    uint32_t clusterNeed = reqSize / CLUSTER_SIZE;
    uint32_t clustBefore = 0;
    uint8_t slotAvailable = 0;
    
    // pembulatan ke atas clusterNeed 
    if(clusterNeed * CLUSTER_SIZE < reqSize){
        clusterNeed++;
    }

    // WRITE FOLDER
    if(request.buffer_size == 0){
        struct FAT32DirectoryTable new_folder;
        init_directory_table(&new_folder, request.name, request.parent_cluster_number);

        for(int i = ROOT_CLUSTER_NUMBER ; i < CLUSTER_SIZE; i++){

            if(driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY ){
                slotAvailable = 1;

                struct FAT32DirectoryEntry new_entry;
                new_entry.user_attribute = UATTR_NOT_EMPTY;
                memcpy(&(new_entry.name), request.name, 8);
                new_entry.cluster_high = (i >> 16) & 0xFFFF;
                new_entry.cluster_low = i & 0xFFFF;
                new_entry.filesize = reqSize;

                for (int j = 0; j < (CLUSTER_SIZE / 32); j++){
                    if(driver_state.dir_table_buf.table[j].user_attribute != UATTR_NOT_EMPTY){
                        driver_state.dir_table_buf.table[j] = new_entry;
                        break;
                    }
                }
                    
                write_clusters(&new_folder, i, 1);

                driver_state.fat_table.cluster_map[i] = FAT32_FAT_END_OF_FILE;
                write_clusters(&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);

                write_clusters(&driver_state.dir_table_buf, request.parent_cluster_number, 1);
                break;
            }
        }
        if (slotAvailable != 1){ // parent dir penuh dan tidak bisa nulis lagi
            return -1;
        }
    }

    else {
        //WRITE FILE
        uint32_t written = 0;

        while(written < clusterNeed){
            for (int i = ROOT_CLUSTER_NUMBER; i < (CLUSTER_SIZE); i++){

                if(driver_state.fat_table.cluster_map[i] == FAT32_FAT_EMPTY_ENTRY ){

                    // di loop pertama akan menulis entry baru ke parent dir entry table
                    if (written == 0){
                        struct FAT32DirectoryEntry new_entry;
                        memset(&new_entry, 0, sizeof(struct FAT32DirectoryEntry));
                        new_entry.user_attribute = UATTR_NOT_EMPTY;
                        memcpy(&(new_entry.name), request.name, 8);
                        memcpy(&(new_entry.ext), request.ext, 3);
                        new_entry.cluster_high = (i >> 16) & 0xFFFF;
                        new_entry.cluster_low = i & 0xFFFF;
                        new_entry.filesize = reqSize;
                        clustBefore = i;

                        for (int j = 0; j < (CLUSTER_SIZE / 32); j++){
                            if(driver_state.dir_table_buf.table[j].user_attribute != UATTR_NOT_EMPTY){
                                driver_state.dir_table_buf.table[j] = new_entry;
                                slotAvailable = 1;
                                break;
                            }
                        }
                        if (slotAvailable != 1){ // parent dir penuh dan tidak bisa nulis lagi
                            return -1;
                        }
                    }

                    struct ClusterBuffer buf_text;
                    memset(buf_text.buf, 0, CLUSTER_SIZE);
                    write_clusters(&buf_text, i, 1);
                    if (written == clusterNeed || clusterNeed == 1){
                        memset(request.buf+(written * CLUSTER_SIZE)+request.buffer_size, i, CLUSTER_SIZE-request.buffer_size);
                    }
                    write_clusters(request.buf+(written * CLUSTER_SIZE), i, 1);

                    if (written > 0){
                        driver_state.fat_table.cluster_map[clustBefore] = i;
                    }
                    
                    if (written == clusterNeed || clusterNeed == 1){
                        driver_state.fat_table.cluster_map[i] = FAT32_FAT_END_OF_FILE;

                        write_clusters(&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
                        write_clusters(&driver_state.dir_table_buf, request.parent_cluster_number, 1);
                        written++;
                        break;
                    }
                    written++;
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

    // request kosong
    if (memcmp(&(request.name), "\0\0\0\0\0\0\0\0", 8) == 0){
        return -1;
    }

    struct FAT32DirectoryTable parent_dir_table;
    uint16_t high, low; 
    int index = -1; 
    read_clusters(&parent_dir_table, request.parent_cluster_number, 1);

    // mencari entry yang akan dihapus di parent dir cluster entry
    for (int i = 0; i < (CLUSTER_SIZE / 32); i++){
        if (memcmp(request.name, parent_dir_table.table[i].name, 8) == 0){
            high = parent_dir_table.table[i].cluster_high;
            low = parent_dir_table.table[i].cluster_low;
            index = i;
            break;
        }
    }

    // entry yang ingin dihapus tidak ditemukan di parent dir cluster entry
    if (index == -1){ 
        return 1;
    }

    // Menghapus entry jika entry adalah folder
    if (memcmp(&(request.ext), "\0\0\0", 3) == 0){
        // int count = 0;
        int req_cluster = (high << 16) | low;
        struct FAT32DirectoryTable *req_dir_table;
        read_clusters(&req_dir_table, req_cluster, 1);

        for (int i = 1; i < (CLUSTER_SIZE / 32); i++){ 
            if (req_dir_table->table[i].user_attribute == UATTR_NOT_EMPTY) {
                return 2;
            }
        }

        parent_dir_table.table[index].user_attribute = 0;
        driver_state.fat_table.cluster_map[req_cluster] = FAT32_FAT_EMPTY_ENTRY;

        write_clusters(&driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
        write_clusters(&parent_dir_table, request.parent_cluster_number, 1);
    }

    // Menghapus entry jika entry adalah file
    else{
        int req_cluster = low | (high << 16);
        uint32_t next;
        // mengupdate FAT table
        do{
            next = driver_state.fat_table.cluster_map[req_cluster];
            driver_state.fat_table.cluster_map[req_cluster] = FAT32_FAT_EMPTY_ENTRY;
            struct ClusterBuffer buf_text;
            memset(buf_text.buf, 0, CLUSTER_SIZE);
            write_clusters(&buf_text, req_cluster, 1);
            req_cluster = next;
        }while(next != FAT32_FAT_END_OF_FILE);
        write_clusters(&driver_state.fat_table, 1, 1);

        // mengupdate parent dir table
        parent_dir_table.table[index].user_attribute = 0;
        memset(parent_dir_table.table[index].name, 0, 32);
        write_clusters(&parent_dir_table, request.parent_cluster_number, 1);
        
        return 0;
    }

    return -1;
}