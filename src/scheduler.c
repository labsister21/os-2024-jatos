#include "header/scheduler/scheduler.h"


// /**
//  * Read all general purpose register values and set control register.
//  * Resume the execution flow back to ctx.eip and ctx.eflags
//  * 
//  * @note          Implemented in assembly
//  * @param context Target context to switch into
//  */
// __attribute__((noreturn)) extern void process_context_switch(struct Context ctx)


/* --- Scheduler --- */
/**
 * Initialize scheduler before executing init process 
 * 
 */

// struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX];

void scheduler_init(void){  
    activate_timer_interrupt();
}

/**
 * Save context to current running process
 * 
 * @param ctx Context to save to current running process control block
 */
void scheduler_save_context_to_current_running_pcb(struct Context ctx){
    struct ProcessControlBlock *current_running_pcb = process_get_current_running_pcb_pointer();

    if (current_running_pcb != NULL){
        // current_running_pcb->metadata.state = PROCESS_STATE_RUNNING;
        current_running_pcb->context = ctx;
    }

}

/**
 * Trigger the scheduler algorithm and context switch to new process
 */
__attribute__((noreturn)) void scheduler_switch_to_next_process(void){
    
    struct ProcessControlBlock *current_running_pcb = process_get_current_running_pcb_pointer();

    if (current_running_pcb == NULL){
       current_running_pcb = _process_list;
       current_running_pcb->metadata.state = PROCESS_STATE_RUNNING;
    } 

    int idx_run = 0;

    for (int i = 0; i < PROCESS_COUNT_MAX; i++){
        if (&_process_list[i] == current_running_pcb){
            idx_run = i;
            break;
        }
    }
    
    int idx_next = idx_run + 1 < PROCESS_COUNT_MAX ? idx_run + 1 : 0;

    while (_process_list[idx_next].metadata.state != PROCESS_STATE_READY && idx_next != idx_run)
    {
       idx_next = idx_next + 1 < PROCESS_COUNT_MAX ? idx_next + 1 : 0;
    }
    
    current_running_pcb->metadata.state = PROCESS_STATE_READY;
    _process_list[idx_next].metadata.state = PROCESS_STATE_RUNNING;
    paging_use_page_directory(_process_list[idx_next].context.page_directory_virtual_addr);
    process_context_switch(_process_list[idx_next].context);
}