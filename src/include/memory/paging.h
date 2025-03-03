#include <stdint.h>
#include <stdbool.h>


#define KERNEL_PFLAG_PRESENT 0b1
#define KERNEL_PFLAG_WRITE   0b10
#define KERNEL_PFLAG_USER    0b100
#define KERNEL_PFLAG_PXD     0b10000000000000000000000000000000000000000000000000000000000000 

#define PAGE_SIZE 4096
#define KERNEL_STACK_PAGES 2LL
#define KERNEL_STACK_PTR 0xFFFFFFFFFFFFF000LL
#define KERNEL_STACK_ADDR KERNEL_STACK_PTR-(KERNEL_STACK_PAGES*PAGE_SIZE)

#define USER_STACK_PAGES 2LL
#define USER_STACK_ADDR (USER_STACK_PTR - USER_STACK_PAGES*PAGE_SIZE)
#define USER_STACK_PTR 0x700000000000LL

#define KERNEL_SWITCH_PAGE_TREE(TREE_ADDRESS) \
    __asm__ volatile (\
       "movq %0, %%cr3"\
       :\
       :  "r" (TREE_ADDRESS)\
    ); \
//    printk(GFX_BLUE,"CR3 switched to 0x%llx\n", TREE_ADDRESS); 

#define KERNEL_SWITCH_STACK(STACK_PTR) \
    __asm__ volatile (\
       "movq %0, %%rsp\n"\
       "movq $0, %%rbp\n"\
       "push $0"\
       :\
       :  "r" (STACK_PTR)\
    ); \
//    printk(GFX_BLUE,"Stack switched to 0x%llx\n", STACK_PTR);


#define switch_page_structures() \
    KERNEL_SWITCH_PAGE_TREE(osinfo.cr3); \
    KERNEL_SWITCH_STACK(KERNEL_STACK_PTR); \

inline bool is5lvl() {
    uint64_t cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    return cr4 & 0x200;
}

void push_vmem(uint64_t* pml4_addr, uint64_t rsp, char* data, size_t len);
void write_vmem(uint64_t* pml4_addr, uint64_t virt_addr, char* data, size_t len);
uint64_t virt_to_phys(uint64_t pml4_addr[], uint64_t virt_addr);
void map_pages(uint64_t pml4_addr[], uint64_t virt_addr, uint64_t phys_addr, uint64_t num_pages, uint64_t flags);
void alloc_pages(uint64_t pml4_addr[], uint64_t virt_addr, uint64_t num_pages, uint64_t flags);
void clear_page_cache(uint64_t addr);
void map_sections(uint64_t pml4[]);
void map_kernel(uint64_t pml4[]);
void map_all(uint64_t pml4[]);
uint64_t* init_paging_task(void);
void init_paging(void);