#include <stdint.h>
#include <stddef.h>
#include <reg.h>

struct PCB{
    uint64_t pid;
    uint64_t rflags;
    uint64_t cr3;
    struct reg_state_t reg_state;
};


int set_state(struct PCB* pcb, struct reg_state_t* reg_state){
    
    return 0;
}