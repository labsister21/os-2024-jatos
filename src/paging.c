#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/memory/paging.h"

__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        },
        [0x300] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        },
    }
};

static struct PageManagerState page_manager_state = {
    .page_frame_map = {
        [0]                            = true,
        [1 ... PAGE_FRAME_MAX_COUNT-1] = false
    },
    .free_page_frame_count = PAGE_FRAME_MAX_COUNT - 1,
    // TODO: Initialize page manager state properly
};

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlag flag
) {
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag          = flag;
    page_dir->table[page_index].lower_address = ((uint32_t) physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr) {
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}



/* --- Memory Management --- */
// TODO: Implement
bool paging_allocate_check(uint32_t amount) {
    // TODO: Check whether requested amount is available
    uint32_t count = 0;
    for (int i = 0; i < PAGE_ENTRY_COUNT; i++){
        if (_paging_kernel_page_directory.table[i].flag.present_bit == 0){
            count++;
        }
    }
    
    // convert ke byte(?) karena page sizenya 4 MB, jdi utk setiap count dikali 2^22(?)
    if ((count << 22) >= amount){
        return true;
    } else {
        return false;
    }
    return true;
}


bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /*
     * TODO: Find free physical frame and map virtual frame into it
     * - Find free physical frame in page_manager_state.page_frame_map[] using any strategies
     * - Mark page_manager_state.page_frame_map[]
     * - Update page directory with user flags:
     *     > present bit    true
     *     > write bit      true
     *     > user bit       true
     *     > pagesize 4 mb  true
     */ 
    int idx = -1;

    for (int i = 0; i < PAGE_FRAME_MAX_COUNT; i++){
        if (!page_manager_state.page_frame_map[i]){
            idx = i;
            break;
        }
    }

    uint32_t phys_address = (idx * (1 << 22)) + 0x400000;

    page_manager_state.page_frame_map[idx] = false;
    page_manager_state.free_page_frame_count--;

    // page_dir->table[virtual_addr].flag.present_bit = 1;
    // page_dir->table[virtual_addr].flag.write_bit = 1;
    // page_dir->table[virtual_addr].flag.user_bit = 1;
    // page_dir->table[virtual_addr].flag.use_pagesize_4_mb = 1;
    // page_dir->table[virtual_addr].higher_address = phys_address diapain?;
    // page_dir->table[virtual_addr].lower_address = phys_address diapain?;

    return true;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /* 
     * TODO: Deallocate a physical frame from respective virtual address
     * - Use the page_dir.table values to check mapped physical frame
     * - Remove the entry by setting it into 0
     */
    return true;
}