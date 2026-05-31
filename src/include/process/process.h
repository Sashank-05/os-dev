#pragma once

#include <stdint.h>
#include <sch/sch.h>

struct user_process {
    uint64_t entry;
    uint64_t user_rsp;
    uint64_t cr3;
};

int process_load(const char* path, struct user_process* process);
void process_bootstrap(void);
int process_launch(const char* path);

// Scheduler integration
pcb_t* process_create_and_schedule(const char* path);