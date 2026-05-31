#include <sch/sch.h>
#include <memory/pmm.h>
#include <memory/kheap.h>
#include <debug/debug.h>
#include <debug/panic.h>
#include <libk/string.h>
#include <common/stdinfo.h>

extern struct OSinfo osinfo;

// Scheduler state
static struct {
    pcb_t* ready_queue;         // Head of ready queue (doubly-linked list)
    pcb_t* current_process;     // Currently running process
    uint64_t next_pid;
    uint64_t process_count;
    uint64_t tick_count;
} scheduler_state = {
    .ready_queue = NULL,
    .current_process = NULL,
    .next_pid = 1,
    .process_count = 0,
    .tick_count = 0,
};

#define DEFAULT_TIME_SLICE 10
#define MAX_PRIORITY 255

// Initialize the scheduler
void scheduler_init(void) {
    scheduler_state.ready_queue = NULL;
    scheduler_state.current_process = NULL;
    scheduler_state.next_pid = 1;
    scheduler_state.process_count = 0;
    scheduler_state.tick_count = 0;
    debug("Scheduler initialized\n");
}

// Create a new process
pcb_t* process_create(uint64_t entry, uint64_t cr3, uint64_t rsp) {
    // Allocate PCB from kernel heap
    pcb_t* pcb = (pcb_t*)kmalloc(sizeof(pcb_t));
    if (!pcb) {
        debug("Failed to allocate PCB\n");
        return NULL;
    }

    memset(pcb, 0, sizeof(pcb_t));
    
    pcb->pid = scheduler_state.next_pid++;
    pcb->state = PROCESS_READY;
    pcb->time_slice = DEFAULT_TIME_SLICE;
    pcb->priority = 128;  // Default priority
    pcb->cr3 = cr3;
    pcb->rflags = 0x202;  // Interrupts enabled, IF flag set
    
    // Set initial register state
    pcb->reg_state.rip = entry;
    pcb->reg_state.rsp = rsp;
    pcb->reg_state.cr3 = cr3;
    pcb->reg_state.rflags = 0x202;
    
    pcb->next = NULL;
    pcb->prev = NULL;
    
    scheduler_state.process_count++;
    debug("Process created: PID=%llu at RIP=0x%llx\n", pcb->pid, entry);
    
    return pcb;
}

// Destroy a process
void process_destroy(pcb_t* pcb) {
    if (!pcb) return;
    
    // Remove from ready queue if present
    process_remove_ready(pcb);
    
    // Note: We don't free memory here as we'd need a full memory allocator
    // In a real OS, this would properly free the process's memory, page tables, etc.
    
    scheduler_state.process_count--;
    debug("Process destroyed: PID=%llu\n", pcb->pid);
}

// Add process to ready queue (round-robin insertion at tail)
void process_add_ready(pcb_t* pcb) {
    if (!pcb) return;
    
    pcb->state = PROCESS_READY;
    
    if (!scheduler_state.ready_queue) {
        // Empty queue
        scheduler_state.ready_queue = pcb;
        pcb->next = pcb;
        pcb->prev = pcb;  // Circular
    } else {
        // Insert at tail
        pcb_t* tail = scheduler_state.ready_queue->prev;
        pcb->next = scheduler_state.ready_queue;
        pcb->prev = tail;
        tail->next = pcb;
        scheduler_state.ready_queue->prev = pcb;
    }
}

// Remove process from ready queue
void process_remove_ready(pcb_t* pcb) {
    if (!pcb || pcb->state == PROCESS_TERMINATED) return;
    
    if (scheduler_state.ready_queue == pcb) {
        if (pcb->next == pcb) {
            // Only process in queue
            scheduler_state.ready_queue = NULL;
        } else {
            scheduler_state.ready_queue = pcb->next;
        }
    }
    
    if (pcb->next) pcb->next->prev = pcb->prev;
    if (pcb->prev) pcb->prev->next = pcb->next;
    
    pcb->next = NULL;
    pcb->prev = NULL;
}

// Get current running process
pcb_t* scheduler_get_current(void) {
    return scheduler_state.current_process;
}

// Set process state
void process_set_state(pcb_t* pcb, process_state_t state) {
    if (!pcb) return;
    
    if (state == PROCESS_READY && pcb->state != PROCESS_READY) {
        process_add_ready(pcb);
    } else if (state != PROCESS_READY && pcb->state == PROCESS_READY) {
        process_remove_ready(pcb);
    }
    
    pcb->state = state;
}

// Get process state
process_state_t process_get_state(pcb_t* pcb) {
    return pcb ? pcb->state : PROCESS_TERMINATED;
}

// Scheduler yield (after timer interrupt)
void scheduler_yield(void) {
    scheduler_switch(NULL);
}

// Context switch - called from timer interrupt or yield
// If current_state is non-NULL, save the state of the current process
void scheduler_switch(struct reg_state_t* current_state) {
    if (!scheduler_state.ready_queue) {
        // No processes to run
        debug("No processes in ready queue\n");
        return;
    }
    
    // Save current process state if provided
    if (scheduler_state.current_process && current_state) {
        scheduler_state.current_process->reg_state = *current_state;
    }
    
    // Round-robin: move current to next
    if (scheduler_state.current_process) {
        // Rotate the queue if time slice expired
        if (scheduler_state.current_process->state == PROCESS_RUNNING) {
            scheduler_state.current_process->state = PROCESS_READY;
            scheduler_state.ready_queue = scheduler_state.ready_queue->next;
        }
    }
    
    // Pick next process from ready queue
    scheduler_state.current_process = scheduler_state.ready_queue;
    if (scheduler_state.current_process) {
        scheduler_state.current_process->state = PROCESS_RUNNING;
        scheduler_state.current_process->time_slice = DEFAULT_TIME_SLICE;
        debug("Context switch to PID=%llu (RIP=0x%llx)\n", 
              scheduler_state.current_process->pid,
              scheduler_state.current_process->reg_state.rip);
    }
}

// Get current PID
uint64_t process_get_pid(void) {
    return scheduler_state.current_process ? scheduler_state.current_process->pid : 0;
}

// Dump scheduler statistics
void scheduler_dump_stats(void) {
    debug("=== Scheduler Statistics ===\n");
    debug("Current PID: %llu\n", process_get_pid());
    debug("Total processes: %llu\n", scheduler_state.process_count);
    debug("Ticks: %llu\n", scheduler_state.tick_count);
    
    uint64_t ready_count = 0;
    pcb_t* iter = scheduler_state.ready_queue;
    if (iter) {
        do {
            ready_count++;
            iter = iter->next;
        } while (iter != scheduler_state.ready_queue);
    }
    debug("Ready processes: %llu\n", ready_count);
}