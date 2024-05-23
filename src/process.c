#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
#include "header/filesystem/fat32.h"




/*DECLARE PROCESS MANAGER STATE*/

struct ProcessControlManagerState process_manager_state = {
    .list_of_process = {false},
    .active_process_count = 0,
};



struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX] = {0};

/**
 * Get currently running process PCB pointer
 * 
 * @return Will return NULL if there's no running process
 */
struct ProcessControlBlock* process_get_current_running_pcb_pointer(void){
    struct ProcessControlBlock *pcb = NULL;
    for (int i = 0; i < PROCESS_COUNT_MAX; i++){
        if (_process_list[i].metadata.state == PROCESS_STATE_RUNNING){
            pcb = &_process_list[i];
            break;
        }
    }
    return pcb;
}


int32_t process_list_get_inactive_index(void){
    for (int i = 0; i < PROCESS_COUNT_MAX; i++){
        if (process_manager_state.list_of_process[i] == false){
            return i;
        }
    }
    return -1;
}

int32_t process_create_user_process(struct FAT32DriverRequest request)
{   
    int32_t retcode = PROCESS_CREATE_SUCCESS;
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX)
    {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t)request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE)
    {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX)
    {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // Process PCB
    int32_t p_index = process_manager_state.active_process_count;
    struct ProcessControlBlock *new_pcb = &(_process_list[p_index]);
    
    /*buat virtual address space baru*/
    struct PageDirectory *page_dir = paging_create_new_page_directory();

    /*baca dan load exe dari filesystem ke memory baru*/
        /*simpan page dir lama*/
        struct PageDirectory *old_page_dir = paging_get_current_page_directory_addr();

        /*load page dir baru*/
        paging_use_page_directory(page_dir);


        paging_allocate_user_page_frame(page_dir, request.buf);
        paging_allocate_user_page_frame(page_dir,(void*) 0xBFFFFFFC);

        /*read req buf buat masukin kode process ke page dir baru*/
        read(request);


        /*kembalikan page dir lama*/
        paging_use_page_directory(old_page_dir);

    /*menyiapkan state & context baru*/
        /*set state process */
        new_pcb->metadata.state = PROCESS_STATE_READY;

        /*set context process*/
        new_pcb->context.cpu.segment.ds = GDT_USER_DATA_SEGMENT_SELECTOR | 0x3;
        new_pcb->context.cpu.segment.es = GDT_USER_DATA_SEGMENT_SELECTOR | 0x3;
        new_pcb->context.cpu.segment.fs = GDT_USER_DATA_SEGMENT_SELECTOR | 0x3;
        new_pcb->context.cpu.segment.gs = GDT_USER_DATA_SEGMENT_SELECTOR | 0x3;
        new_pcb->context.cpu.stack.ebp = 0xBFFFFFFC;
        new_pcb->context.cpu.stack.esp = 0xBFFFFFFC;
        new_pcb->context.eip = (uint32_t)request.buf;
        new_pcb->context.eflags |= CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;
        new_pcb->context.page_directory_virtual_addr = page_dir;



    /*mencatat semua informasi penting ke metadata pcb*/
    memset(new_pcb->metadata.name, 0, 8);
    memcpy(new_pcb->metadata.name, request.name, 8);
    new_pcb->metadata.pid = p_index;

    process_manager_state.active_process_count++;


    /*mengembalikan state register ke semula*/

    


exit_cleanup:
    return retcode;
}

void process_list_release(uint32_t index){
    _process_list[index].metadata.state = PROCESS_STATE_READY;
    _process_list[index].metadata.pid = 0;
    _process_list[index].memory.page_frame_used_count = 0;
    memset(_process_list[index].memory.virtual_addr_used, 0, sizeof(_process_list[index].memory.virtual_addr_used));
}

void paging_release_page_directory(uint32_t pid){
    struct PageDirectory *page_dir = _process_list[pid].context.page_directory_virtual_addr;
    paging_free_page_directory(page_dir);
}

/**
 * Destroy process then release page directory and process control block
 * 
 * @param pid Process ID to delete
 * @return    True if process destruction success
 */
bool process_destroy(uint32_t pid){
    if (pid >= PROCESS_COUNT_MAX){
        return false;
    }
    if(process_manager_state.list_of_process[pid]){
        return false;
    }

    struct ProcessControlBlock *pcb = NULL;
    for (int i = 0; i < PROCESS_COUNT_MAX; i++){
        if (_process_list[i].metadata.pid == pid){
            pcb = &_process_list[i];
            break;
        }
    }

    struct PageDirectory *page_dir = pcb->context.page_directory_virtual_addr;
    paging_free_page_directory(page_dir);

    process_manager_state.list_of_process[pid] = false;
    _process_list[pid] = (struct ProcessControlBlock){
          .metadata.state = NO_PROCESS,
    };
    
    memset(_process_list[pid].metadata.name, 0, 8);
    process_manager_state.active_process_count--;
    return true;
}