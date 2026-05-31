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
    SYS_ps = 8,
    SYS_net_recv = 9,
    SYS_netstat = 10,
};

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
    return syscall3(SYS_write, (long)text, (long)length, 0);
}

static long sys_readline(char* buffer, size_t size) {
    return syscall3(SYS_readline, (long)buffer, (long)size, 0);
}

static long sys_ticks(void) {
    return syscall3(SYS_ticks, 0, 0, 0);
}

static void sys_exit(long code) {
    syscall3(SYS_exit, code, 0, 0);
    for (;;) {
        asm volatile("hlt");
    }
}

static int strcmp_local(const char* a, const char* b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static int strncmp_local(const char* a, const char* b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return (unsigned char)a[i] - (unsigned char)b[i];
        }
        if (!a[i]) return 0;
    }
    return 0;
}

static size_t strlen_local(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

struct dir_entry {
    char name[256];
    uint32_t type;  // 1=file, 2=dir
};

static long sys_listdir(const char* path, struct dir_entry* entries, size_t max_entries) {
    return syscall3(SYS_listdir, (long)path, (long)entries, (long)max_entries);
}

static long sys_readfile(const char* path, char* buffer, size_t size) {
    return syscall3(SYS_readfile, (long)path, (long)buffer, (long)size);
}

static long sys_exec(const char* path) {
    return syscall3(SYS_exec, (long)path, 0, 0);
}

struct process_info {
    uint64_t pid;
    uint64_t rip;
    uint32_t state;  
};

static long sys_ps(struct process_info* processes, size_t max_count) {
    return syscall3(SYS_ps, (long)processes, (long)max_count, 0);
}

static long sys_net_send(const char* data, size_t len) {
    return syscall3(SYS_net_send, (long)data, (long)len, 0);
}

static long sys_net_recv(char* buffer, size_t max_len) {
    return syscall3(SYS_net_recv, (long)buffer, (long)max_len, 0);
}

static long sys_netstat(void) {
    return syscall3(SYS_netstat, 0, 0, 0);
}

static void clear_screen(void) {
    // Try ANSI clear+home first.
    sys_write("\x1b[2J\x1b[H");
    // Fallback for simple console paths.
    for (int i = 0; i < 40; i++) {
        sys_write("\n");
    }
}

static void print_prompt(void) {
    sys_write("root@sas-os# ");
}

static void print_help(void) {
    sys_write("=== SAS OS Shell ===\n");
    sys_write("help       - show this help\n");
    sys_write("clear      - clear terminal output\n");
    sys_write("neofetch   - show system summary\n");
    sys_write("uname      - show OS name\n");
    sys_write("whoami     - show current user\n");
    sys_write("pwd        - print working directory\n");
    sys_write("uptime     - show uptime\n");
    sys_write("echo TEXT  - print text\n");
    sys_write("ls [PATH]  - list files/folders\n");
    sys_write("cat FILE   - view file contents\n");
    sys_write("exec FILE  - execute ELF binary\n");
    sys_write("ps         - list running processes\n");
    sys_write("netstat    - show network statistics\n");
    sys_write("netsend X  - send text on net interface\n");
    sys_write("netrecv    - receive text from net interface\n");
    sys_write("ticks      - show timer ticks\n");
    sys_write("exit       - leave userspace\n");
}

static void print_number(long value) {
    char buffer[32];
    int index = 0;

    if (value == 0) {
        sys_write("0");
        return;
    }

    if (value < 0) {
        sys_write("-");
        value = -value;
    }

    while (value > 0 && index < (int)sizeof(buffer)) {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index-- > 0) {
        char c = buffer[index];
        syscall3(SYS_write, (long)&c, 1, 0);
    }
}

static void run_command(char* line) {
    // Trim leading whitespace
    while (*line == ' ' || *line == '\t') line++;
    
    if (strcmp_local(line, "help") == 0) {
        print_help();
        return;
    }

    if (strcmp_local(line, "clear") == 0) {
        clear_screen();
        return;
    }

    if (strcmp_local(line, "uname") == 0) {
        sys_write("SAS OS\n");
        return;
    }

    if (strcmp_local(line, "whoami") == 0) {
        sys_write("root\n");
        return;
    }

    if (strcmp_local(line, "pwd") == 0) {
        sys_write("/\n");
        return;
    }

    if (strcmp_local(line, "uptime") == 0) {
        long ticks = sys_ticks();
        long seconds = ticks / 100;  // PIT configured to 100 Hz in kernel
        long mins = seconds / 60;
        long secs = seconds % 60;

        sys_write("uptime: ");
        print_number(mins);
        sys_write("m ");
        print_number(secs);
        sys_write("s\n");
        return;
    }

    if (strcmp_local(line, "neofetch") == 0) {
        long ticks = sys_ticks();
        long seconds = ticks / 100;
        long mins = seconds / 60;
        long secs = seconds % 60;

        sys_write("==============================\n");
        sys_write("  SSS    AAA    SSS           \n");
        sys_write(" S      A   A  S              \n");
        sys_write("  SSS   AAAAA   SSS           \n");
        sys_write("     S  A   A      S          \n");
        sys_write(" SSSS   A   A  SSSS           \n");
        sys_write("                              \n");
        sys_write("Brand   : SAS\n");
        sys_write("OS      : SAS OS\n");
        sys_write("User    : root\n");
        sys_write("Shell   : sas-sh\n");
        sys_write("Uptime  : ");
        print_number(mins);
        sys_write("m ");
        print_number(secs);
        sys_write("s\n");
        sys_write("Ticks   : ");
        print_number(ticks);
        sys_write("\n");
        sys_write("==============================\n");
        return;
    }

    if (strcmp_local(line, "ticks") == 0) {
        sys_write("ticks=");
        print_number(sys_ticks());
        sys_write("\n");
        return;
    }

    if (strcmp_local(line, "exit") == 0) {
        sys_exit(0);
    }

    // ls [path] command
    if (strncmp_local(line, "ls", 2) == 0 && (line[2] == ' ' || line[2] == '\0')) {
        char* path = line + 2;
        while (*path == ' ' || *path == '\t') path++;
        if (*path == '\0') {
            path = "/";
        }

        struct dir_entry entries[16];  // Reduced from 64
        long count = sys_listdir(path, entries, 16);  // Reduced from 64
        
        if (count < 0) {
            sys_write("ls: cannot access '");
            sys_write(path);
            sys_write("'\n");
        } else if (count == 0) {
            sys_write("(empty)\n");
        } else {
            for (long i = 0; i < count; i++) {
                if (entries[i].type == 2) {
                    sys_write("[D] ");
                } else {
                    sys_write("[F] ");
                }
                sys_write(entries[i].name);
                sys_write("\n");
            }
        }
        return;
    }

    // cat file command
    if (strncmp_local(line, "cat", 3) == 0 && line[3] == ' ') {
        char* path = line + 4;
        while (*path == ' ' || *path == '\t') path++;
        
        if (*path == '\0') {
            sys_write("cat: missing filename\n");
            return;
        }

        char buffer[512];  // Reduced from 4096
        long read = sys_readfile(path, buffer, sizeof(buffer) - 1);
        
        if (read < 0) {
            sys_write("cat: cannot open '");
            sys_write(path);
            sys_write("'\n");
        } else if (read > 0) {
            buffer[read] = '\0';
            sys_write(buffer);
            if (buffer[read - 1] != '\n') {
                sys_write("\n");
            }
        }
        return;
    }

    // exec file command
    if (strncmp_local(line, "exec", 4) == 0 && line[4] == ' ') {
        char* path = line + 5;
        while (*path == ' ' || *path == '\t') path++;
        
        if (*path == '\0') {
            sys_write("exec: missing filename\n");
            return;
        }

        long result = sys_exec(path);
        if (result < 0) {
            sys_write("exec: cannot execute '");
            sys_write(path);
            sys_write("'\n");
        }
        return;
    }

    // echo text command
    if (strncmp_local(line, "echo", 4) == 0 && line[4] == ' ') {
        sys_write(line + 5);
        sys_write("\n");
        return;
    }

    // ps command - list running processes
    if (strcmp_local(line, "ps") == 0) {
        struct process_info processes[32];
        long count = sys_ps(processes, 32);
        
        if (count < 0) {
            sys_write("ps: error getting process list\n");
        } else if (count == 0) {
            sys_write("(no processes)\n");
        } else {
            sys_write("PID     RIP              STATE\n");
            sys_write("---     ---              -----\n");
            for (long i = 0; i < count; i++) {
                print_number(processes[i].pid);
                sys_write("      0x");
                
                // Print RIP in hex
                char hex_buf[16];
                long val = processes[i].rip;
                int hex_idx = 0;
                if (val == 0) {
                    sys_write("0");
                } else {
                    while (val > 0 && hex_idx < 15) {
                        int digit = val % 16;
                        hex_buf[hex_idx++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
                        val /= 16;
                    }
                    while (hex_idx-- > 0) {
                        char c = hex_buf[hex_idx];
                        syscall3(SYS_write, (long)&c, 1, 0);
                    }
                }
                
                sys_write("  ");
                const char* state_str = "unknown";
                switch (processes[i].state) {
                    case 0: state_str = "ready"; break;
                    case 1: state_str = "running"; break;
                    case 2: state_str = "blocked"; break;
                    case 3: state_str = "terminated"; break;
                }
                sys_write(state_str);
                sys_write("\n");
            }
        }
        return;
    }

    // netstat command - network statistics
    if (strcmp_local(line, "netstat") == 0) {
        long result = sys_netstat();
        if (result < 0) {
            sys_write("netstat: no network interface available\n");
        }
        return;
    }

    if (strncmp_local(line, "netsend", 7) == 0 && line[7] == ' ') {
        char* payload = line + 8;
        while (*payload == ' ' || *payload == '\t') payload++;
        if (*payload == '\0') {
            sys_write("netsend: missing payload\n");
            return;
        }
        long sent = sys_net_send(payload, strlen_local(payload));
        if (sent < 0) {
            sys_write("netsend: failed\n");
        } else {
            sys_write("netsend: bytes=");
            print_number(sent);
            sys_write("\n");
        }
        return;
    }

    if (strcmp_local(line, "netrecv") == 0) {
        char buffer[256];
        long got = sys_net_recv(buffer, sizeof(buffer) - 1);
        if (got <= 0) {
            sys_write("netrecv: no data\n");
            return;
        }
        buffer[got] = '\0';
        sys_write(buffer);
        sys_write("\n");
        return;
    }

    sys_write("command not found: ");
    sys_write(line);
    sys_write("\n");
}

__attribute__((noreturn)) void _start(void) {
    char line[256];

    sys_write("SAS OS\n");
    sys_write("type help for commands\n");

    for (;;) {
        print_prompt();
        long read = sys_readline(line, sizeof(line));
        
        // Only process if we actually got input
        if (read > 0) {
            // Trim null terminator from count if present
            if (line[read - 1] == '\0') {
                read--;
            }
            
            // Only run if there's actual content
            if (read > 0) {
                line[read] = '\0';  // Ensure null termination
                run_command(line);
            }
        }
    }
}
