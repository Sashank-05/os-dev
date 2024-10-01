#define VGA_START 0xB8000
#define VGA_EXTENT (80 * 25)
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

void putss(char *str);
void clear();
void clear_line(int line);
void boot_print(char *str);
void boot_print_clear();
void showboot();
void boot_println(char *str);
void boot_backspace();
