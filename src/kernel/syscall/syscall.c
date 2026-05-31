#include <syscall/syscall.h>

#include <common/stdinfo.h>
#include <debug/debug.h>
#include <drivers/keyboard.h>
#include <drivers/serial.h>
#include <drivers/framebuffer.h>
#include <fs/vfs.h>
#include <process/process.h>
#include <libk/string.h>
#include <net/net.h>

extern volatile uint64_t irq_ticks;

static struct {
    uint8_t valid;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
    uint64_t cr3;
} exec_parent_ctx = {0};

struct dir_entry {
    char name[256];
    uint32_t type;  // 1=file, 2=dir
};

static uint64_t syscall_write(const char* buffer, uint64_t length) {
    if (!buffer || length == 0) {
        return 0;
    }

    char temp[1025];
    size_t copy_length = length < sizeof(temp) - 1 ? (size_t)length : sizeof(temp) - 1;
    memcpy(temp, buffer, copy_length);
    temp[copy_length] = '\0';

    debug("%s", temp);
    serial_print(COM1, "%s", temp);
    printk(GFX_WHITE, "%s", temp);
    return copy_length;
}

static uint64_t syscall_net_send(const uint8_t* data, uint64_t length) {
    return net_send("lo0", data, (size_t)length);
}

static int64_t syscall_listdir(const char* path, struct dir_entry* entries, size_t max_entries) {
    if (!path || !entries || max_entries == 0) {
        return -1;
    }

    size_t vfs_count_total = vfs_count();
    size_t entry_count = 0;
    size_t path_len = strlen(path);
    
    // Normalize path
    if (path_len > 1 && path[path_len - 1] == '/') {
        path_len--;
    }

    for (size_t i = 0; i < vfs_count_total && entry_count < max_entries; i++) {
        const struct vfs_node* node = vfs_find_by_index(i);
        if (!node) continue;

        // Check if node is in this directory
        const char* node_path = node->path;
        if (path_len == 0 || (path[0] == '/' && path_len == 1)) {
            // Root directory - show top-level items
            char* slash = strchr(node_path, '/');
            if (!slash) {
                // Top-level file
                memset(&entries[entry_count], 0, sizeof(struct dir_entry));
                size_t len = strlen(node_path);
                if (len >= sizeof(entries[entry_count].name)) {
                    len = sizeof(entries[entry_count].name) - 1;
                }
                memcpy(entries[entry_count].name, node_path, len);
                entries[entry_count].name[len] = '\0';
                entries[entry_count].type = (node->type == VFS_NODE_DIR) ? 2 : 1;
                entry_count++;
            }
        } else {
            // Check if file is in the specified directory
            if (strncmp(node_path, path, path_len) == 0 && node_path[path_len] == '/') {
                const char* relative = node_path + path_len + 1;
                char* slash = strchr(relative, '/');
                if (!slash) {
                    // Direct child
                    memset(&entries[entry_count], 0, sizeof(struct dir_entry));
                    size_t len = strlen(relative);
                    if (len >= sizeof(entries[entry_count].name)) {
                        len = sizeof(entries[entry_count].name) - 1;
                    }
                    memcpy(entries[entry_count].name, relative, len);
                    entries[entry_count].name[len] = '\0';
                    entries[entry_count].type = (node->type == VFS_NODE_DIR) ? 2 : 1;
                    entry_count++;
                }
            }
        }
    }

    return (int64_t)entry_count;
}

static int64_t syscall_readfile(const char* path, char* buffer, size_t size, size_t offset) {
    if (!path || !buffer || size == 0) {
        return -1;
    }

    return (int64_t)vfs_read(path, buffer, size, offset);
}

static int64_t syscall_filesize(const char* path) {
    if (!path) return -1;
    const struct vfs_node* node = vfs_find(path);
    if (!node || node->type != VFS_NODE_FILE) return -1;
    return (int64_t)node->size;
}

static int64_t syscall_sleep(uint32_t ms) {
    // Simple busy wait - could be optimized with actual timer
    // Each loop iteration is approximately 1-2us on modern CPUs
    uint32_t iterations = ms * 500;  // Rough estimate
    for (uint32_t i = 0; i < iterations; i++) {
        __asm__ volatile ("nop");
    }
    return 0;
}

struct framebuffer_info {
    uint32_t* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
};

extern struct GFX_Struct gfx;

static int64_t syscall_get_framebuffer(struct framebuffer_info* info) {
    if (!info) {
        return -1;
    }

    info->address = (uint32_t*)gfx.fb_addr;
    info->width = gfx.fb_width;
    info->height = gfx.fb_height;
    info->pitch = gfx.fb_pitch;

    return 0;
}

static int64_t syscall_exec(const char* path, struct interrupt_frame* frame) {
    if (!path) {
        return -1;
    }

    if (exec_parent_ctx.valid) {
        return -1;
    }

    struct user_process process;
    if (!process_load(path, &process)) {
        return -1;
    }

    uint64_t* raw = (uint64_t*)frame;

    exec_parent_ctx.rip = raw[17];
    exec_parent_ctx.cs = raw[18];
    exec_parent_ctx.rflags = raw[19];
    exec_parent_ctx.rsp = raw[20];
    exec_parent_ctx.ss = raw[21];
    __asm__ volatile ("mov %%cr3, %0" : "=r"(exec_parent_ctx.cr3));
    exec_parent_ctx.valid = 1;

    raw[17] = process.entry;
    raw[18] = 0x23;          // CS
    raw[19] |= 0x200;        // RFLAGS
    raw[20] = process.user_rsp;
    raw[21] = 0x1B;          // SS

    __asm__ volatile ("mov %0, %%cr3" : : "r"(process.cr3));
    return 0;
}

void syscall_init(void) {
    debug("Syscall ABI initialized\n");
}

void syscall_handle(struct interrupt_frame* frame) {
    __asm__ volatile ("sti");
    switch (frame->rax) {
    case SYS_write:
        frame->rax = syscall_write((const char*)frame->rdi, frame->rsi);
        break;
    case SYS_exit:
        debug("User process exited with code %llu\n", frame->rdi);
        if (exec_parent_ctx.valid) {
            uint64_t* raw = (uint64_t*)frame;

            raw[17] = exec_parent_ctx.rip;
            raw[18] = exec_parent_ctx.cs;
            raw[19] = exec_parent_ctx.rflags;
            raw[20] = exec_parent_ctx.rsp;
            raw[21] = exec_parent_ctx.ss;
            __asm__ volatile ("mov %0, %%cr3" : : "r"(exec_parent_ctx.cr3));

            exec_parent_ctx.valid = 0;
            frame->rax = 0;
            break;
        }

        while (1) {
            __asm__ volatile ("hlt");
        }
        break;
    case SYS_ticks:
        frame->rax = irq_ticks;
        break;
    case SYS_net_send:
        frame->rax = syscall_net_send((const uint8_t*)frame->rdi, frame->rsi);
        break;
    case SYS_readline:
        frame->rax = keyboard_readline((char*)frame->rdi, (size_t)frame->rsi);
        break;
    case SYS_listdir:
        frame->rax = (uint64_t)syscall_listdir((const char*)frame->rdi, (struct dir_entry*)frame->rsi, (size_t)frame->rdx);
        break;
    case SYS_readfile:
        frame->rax = (uint64_t)syscall_readfile((const char*)frame->rdi, (char*)frame->rsi, (size_t)frame->rdx, (size_t)frame->r10);
        break;
    case SYS_filesize:
        frame->rax = (uint64_t)syscall_filesize((const char*)frame->rdi);
        break;
    case SYS_exec:
        frame->rax = syscall_exec((const char*)frame->rdi, frame);
        break;
    case SYS_sleep:
        frame->rax = syscall_sleep((uint32_t)frame->rdi);
        break;
    case SYS_get_framebuffer:
        frame->rax = syscall_get_framebuffer((struct framebuffer_info*)frame->rdi);
        break;
    case SYS_keyevent: {
        uint8_t pressed = 0;
        uint8_t key = 0;
        if (!keyboard_poll_event(&pressed, &key)) {
            frame->rax = (uint64_t)-1;
            break;
        }
        frame->rax = ((uint64_t)pressed << 8) | (uint8_t)key;
        break;
    }
    default:
        frame->rax = (uint64_t)-1;
        break;
    }
}