#pragma once

#include <stdint.h>
#include <stddef.h>
#include <reg.h>

// Process states
typedef enum {
    PROCESS_READY = 0,
    PROCESS_RUNNING = 1,
    PROCESS_BLOCKED = 2,
    PROCESS_TERMINATED = 3,
} process_state_t;

// Process Control Block (PCB)
typedef struct pcb {
    uint64_t pid;
    process_state_t state;
    struct reg_state_t reg_state;
    uint64_t cr3;               // Page table root
    uint64_t rflags;
    
    // Scheduling info
    uint64_t time_slice;        // Remaining time quantum
    uint64_t priority;          // Priority level (higher = more important)
    
    // Linked list pointers
    struct pcb* next;
    struct pcb* prev;
} pcb_t;

// Scheduler initialization
void scheduler_init(void);

// Process management
pcb_t* process_create(uint64_t entry, uint64_t cr3, uint64_t rsp);
void process_destroy(pcb_t* pcb);
void process_add_ready(pcb_t* pcb);
void process_remove_ready(pcb_t* pcb);

// Scheduling operations
pcb_t* scheduler_get_current(void);
void scheduler_yield(void);
void scheduler_switch(struct reg_state_t* current_state);

// Process state management
void process_set_state(pcb_t* pcb, process_state_t state);
process_state_t process_get_state(pcb_t* pcb);

// Utility
uint64_t process_get_pid(void);
void scheduler_dump_stats(void);
