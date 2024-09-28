void print(const char* str) {
    volatile char* video_memory = (volatile char*)0xb8000;
    while (*str != 0) {
        *video_memory++ = *str++;
        *video_memory++ = 0x0F;  
    }
}

void clear(){
    volatile char* video_memory = (volatile char*)0xb8000;
    for(int i = 0; i < 80*25; i++){
        *video_memory++ = ' ';
        *video_memory++ = 0x0F;
    }
}

void kernel_main() {
    clear();
    print("Hello from 32-bit C kernel!");
    while(1);
}