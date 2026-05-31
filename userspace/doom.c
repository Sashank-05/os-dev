// SAS OS Doom port entry point

#include <stddef.h>
#include <stdint.h>

enum syscall_number {
    SYS_write = 0,
    SYS_exit = 1,
    SYS_ticks = 2,
    SYS_net_send = 3,
    SYS_readline = 4,
    SYS_listdir = 5,
    SYS_readfile = 6,
    SYS_exec = 7,
    SYS_sleep = 8,
    SYS_get_framebuffer = 9,
};

static long syscall0(long number) {
    register long rax asm("rax") = number;
    asm volatile("int $0x80" : "+r"(rax) : : "rcx", "r8", "r9", "r10", "r11", "memory");
    return rax;
}

static long syscall1(long number, long arg1) {
    register long rax asm("rax") = number;
    register long rdi asm("rdi") = arg1;
    asm volatile("int $0x80" : "+r"(rax) : "r"(rdi) : "rcx", "r8", "r9", "r10", "r11", "memory");
    return rax;
}

static long syscall2(long number, long arg1, long arg2) {
    register long rax asm("rax") = number;
    register long rdi asm("rdi") = arg1;
    register long rsi asm("rsi") = arg2;
    asm volatile("int $0x80" : "+r"(rax) : "r"(rdi), "r"(rsi) : "rcx", "r8", "r9", "r10", "r11", "memory");
    return rax;
}

static long syscall3(long number, long arg1, long arg2, long arg3) {
    register long rax asm("rax") = number;
    register long rdi asm("rdi") = arg1;
    register long rsi asm("rsi") = arg2;
    register long rdx asm("rdx") = arg3;
    asm volatile("int $0x80" : "+r"(rax) : "r"(rdi), "r"(rsi), "r"(rdx) : "rcx", "r8", "r9", "r10", "r11", "memory");
    return rax;
}

static long sys_write(const char* text) {
    size_t length = 0;
    while (text[length]) {
        length++;
    }
    return syscall3(SYS_write, (long)text, length, 0);
}

static long sys_sleep(uint32_t ms) {
    return syscall1(SYS_sleep, ms);
}

static uint32_t sys_ticks(void) {
    return (uint32_t)syscall0(SYS_ticks);
}

struct framebuffer_info {
    uint32_t* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;  // bytes per row
};

static struct framebuffer_info sys_get_framebuffer(void) {
    struct framebuffer_info fb;
    syscall1(SYS_get_framebuffer, (long)&fb);
    return fb;
}

void DG_Init() {
    sys_write("DOOM - SAS OS\n");
}

void DG_DrawFrame() {
    // Draw DG_ScreenBuffer to framebuffer
    extern uint32_t* DG_ScreenBuffer;
    struct framebuffer_info fb = sys_get_framebuffer();
    
    if (!fb.address) {
        return;
    }

    // Copy 640x400 32-bit pixels to framebuffer
    uint32_t* src = (uint32_t*)DG_ScreenBuffer;
    uint32_t* dst = fb.address;
    
    for (int y = 0; y < 400; y++) {
        for (int x = 0; x < 640; x++) {
            dst[y * fb.width + x] = src[y * 640 + x];
        }
    }
}

void DG_SleepMs(uint32_t ms) {
    sys_sleep(ms);
}

uint32_t DG_GetTicksMs() {
    return sys_ticks();
}

int DG_GetKey(int* pressed, unsigned char* key) {
    // TODO: Hook into keyboard input
    // For now, no input
    return 0;
}

void DG_SetWindowTitle(const char * title) {
    // Optional - ignore for now
}

int main(int argc, char **argv) {
    extern void doomgeneric_Create(int argc, char **argv);
    extern void doomgeneric_Tick();

    doomgeneric_Create(argc, argv);

    while (1) {
        doomgeneric_Tick();
    }

    return 0;
}
