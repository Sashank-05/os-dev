#include <process/process.h>

#include <common/gdt.h>
#include <common/stdinfo.h>
#include <debug/panic.h>
#include <debug/debug.h>
#include <fs/vfs.h>
#include <libk/string.h>
#include <memory/paging.h>
#include <memory/pmm.h>

extern struct OSinfo osinfo;
extern void enter_user_mode(uint64_t rip, uint64_t rsp, uint64_t cr3);

typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} __attribute__((packed)) Elf64_Phdr;

#define ELF_PT_LOAD 1
#define USER_ELF_ENTRY_PATH "init"

static uint64_t align_down(uint64_t value) {
    return value & ~(PAGE_SIZE - 1);
}

static uint64_t align_up(uint64_t value) {
    return (value + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

int process_load(const char* path, struct user_process* process) {
    const struct vfs_node* file = vfs_find(path);
    if (!file || file->size < sizeof(Elf64_Ehdr)) {
        return 0;
    }

    const Elf64_Ehdr* ehdr = (const Elf64_Ehdr*)file->data;
    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        return 0;
    }

    if (ehdr->e_ident[4] != 2 || ehdr->e_ident[5] != 1) {
        return 0;
    }

    uint64_t* pml4 = init_paging_task();
    if (!pml4) {
        return 0;
    }

    alloc_pages(pml4, USER_STACK_ADDR, USER_STACK_PAGES, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER);

    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        const Elf64_Phdr* phdr = (const Elf64_Phdr*)(file->data + ehdr->e_phoff + (i * ehdr->e_phentsize));
        if (phdr->p_type != ELF_PT_LOAD) {
            continue;
        }

        uint64_t segment_start = align_down(phdr->p_vaddr);
        uint64_t segment_end = align_up(phdr->p_vaddr + phdr->p_memsz);
        uint64_t pages = (segment_end - segment_start) / PAGE_SIZE;
        uint64_t phys = (uint64_t)pmm_alloc_pages(pages);
        if (!phys) {
            return 0;
        }

        map_pages(pml4, segment_start, phys, pages, KERNEL_PFLAG_PRESENT | KERNEL_PFLAG_WRITE | KERNEL_PFLAG_USER);

        void* segment_virt = (void*)(phys + osinfo.hhdm);
        memset(segment_virt, 0, pages * PAGE_SIZE);

        void* destination = (void*)((uint64_t)segment_virt + (phdr->p_vaddr - segment_start));
        memcpy(destination, file->data + phdr->p_offset, phdr->p_filesz);
    }

    process->entry = ehdr->e_entry;
    process->user_rsp = USER_STACK_PTR;
    process->cr3 = (uint64_t)pml4 - osinfo.hhdm;
    return 1;
}

int process_launch(const char* path) {
    struct user_process process;

    if (!process_load(path, &process)) {
        return 0;
    }

    debug("Launching userspace %s at 0x%llx\n", path, process.entry);
    enter_user_mode(process.entry, process.user_rsp, process.cr3);
    return 1;
}

void process_bootstrap(void) {
    struct user_process init_process;

    if (!process_load(USER_ELF_ENTRY_PATH, &init_process)) {
        panic("Failed to load userspace init ELF image\n");
    }

    debug("Launching userspace init at 0x%llx\n", init_process.entry);
    enter_user_mode(init_process.entry, init_process.user_rsp, init_process.cr3);
}


pcb_t* process_create_and_schedule(const char* path) {
    struct user_process process;

    if (!process_load(path, &process)) {
        debug("Failed to load process %s\n", path);
        return NULL;
    }

    pcb_t* pcb = process_create(process.entry, process.cr3, process.user_rsp);
    if (!pcb) {
        debug("Failed to create PCB for %s\n", path);
        return NULL;
    }

    process_add_ready(pcb);
    debug("Process %s scheduled (PID=%llu)\n", path, pcb->pid);
    
    return pcb;
}
