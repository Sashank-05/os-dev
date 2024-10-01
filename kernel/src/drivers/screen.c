#include <drivers/screen.h>

extern int curr_line;

int curr_line = 10;

void clear_line(int line) {
    char *video_memory = (char *)VGA_START;
    for (int i = 0; i < 80; i++) {
        video_memory[(line * 80 + i) * 2] = ' ';  // Use space instead of '\0'
        video_memory[(line * 80 + i) * 2 + 1] = 0xF0; // White text on black background
    }
}

// Function to clear the screen
void clear() {
    char *video_memory = (char *)VGA_START;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video_memory[i] = ' ';  // Clear with spaces
        video_memory[i + 1] = 0xF0; // White text on black background
    }
}

// Function to print a string to the screen
void putss(char *str) {
    char *video_memory = (char *)VGA_START;
    int i = 0;
    while (*str != '\0') {
        video_memory[i] = *str;
        video_memory[i + 1] = 0xF0; // White text on black background
        str++;
        i += 2;
    }
}

// Function to clear the boot print area
void boot_print_clear() {
    char *video_memory = (char *)VGA_START;
    for (int i = 10; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            video_memory[(i * 80 + j) * 2] = ' ';  // Clear with spaces
            video_memory[(i * 80 + j) * 2 + 1] = 0xF0; // White text on black background
        }
    }
    curr_line = 10;
}

// Function to print a string to the screen at a specific line
void boot_print(char *str) {
    char *video_memory = (char *)VGA_START;
    int i = 0;
    int line = curr_line; 
    clear_line(line);
    while (*str != '\0') {
        video_memory[(line * 80 + i) * 2] = *str;
        video_memory[(line * 80 + i) * 2 + 1] = 0xF0;  // White text on black background
        str++;
        i++;
    }
    curr_line++;

    if (curr_line >= 25) {
        boot_print_clear();
        curr_line = 10;
    }
}

// Function to print a string to the screen and append it to the current line
void boot_println(char* str){
    char *video_memory = (char *)VGA_START;
    int i = 0;
    int line = curr_line; 
    clear_line(line);  // Clear the line first

    while (i < 80 && video_memory[(line * 80 + i) * 2] != ' ') {
        i++;  // Find the first empty space (i.e., not ' ')
    }

    while (*str != '\0' && i < 80) {
        video_memory[(line * 80 + i) * 2] = *str;
        video_memory[(line * 80 + i) * 2 + 1] = 0xF0;  // White text on black background
        str++;
        i++;
    }
    

    if (curr_line >= 25) {
        boot_print_clear();
        curr_line = 10;
    }
}

// Function to handle backspace
void boot_backspace(){
    char *video_memory = (char *)VGA_START;
    int line = curr_line;
    int i = 0;

    while (i < 80 && video_memory[(line * 80 + i) * 2] != ' ') {
        i++;  // Find the last character in the line
    }

    if (i > 0) {
        i--;  // Move to the last printed character
        video_memory[(line * 80 + i) * 2] = ' ';  // Clear the last character by overwriting with a space
        video_memory[(line * 80 + i) * 2 + 1] = 0x0F; // Reset the attribute (white text on black background)
    }
}

// Function to show the boot screen
void showboot() {
    char *video_memory = (char *)VGA_START;

    const char *boot_message[] = {
        "   _____               _____      ____     _____ ",
        "  / ____|     /\\      / ____|    / __ \\   / ____|",
        " | (___      /  \\    | (___     | |  | | | (___  ",
        "  \\___ \\    / /\\ \\    \\___ \\    | |  | |  \\___ \\ ",
        "  ____) |  / ____ \\   ____) |   | |__| |  ____) |",
        " |_____/  /_/    \\_\\ |_____/     \\____/  |_____/ "
    };

    // Calculate starting positions to center the boot message
    int start_x = (80 - 49) / 2;
    int start_y = 2;

    // Clear previous content
    clear();

    // Write the boot message to the video memory
    for (int y = 0; y < 6; y++) {
        for (int x = 0; x < 49; x++) {
            video_memory[((start_y + y) * 80 + (start_x + x)) * 2] = boot_message[y][x];
            video_memory[((start_y + y) * 80 + (start_x + x)) * 2 + 1] = 0xF0;  
        }
    }

    // Can start printing from line 10
    boot_print("[+] Booting...");
}
