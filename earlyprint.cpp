#include "earlyprint.h"

static uint32 xpos, ypos;

static void incpos() {
    xpos++;
    if(xpos == 80) {
        xpos = 0;
        ypos++;
    }
}

static void putc(char c) {
    if(c == '\n') {
        xpos = 0;
        ypos++;
        return;
    }
    unsigned char *videoram = (unsigned char *) 0xb8000;
    uint32 vrampos = ypos*80 + xpos;
    videoram[vrampos*2] = c;
    videoram[vrampos*2+1] = 0x07;
    incpos();
}

void earlycls() {
    unsigned char *videoram = (unsigned char *) 0xb8000;
    for (int i = 0; i < 80*25; i++) {
        videoram[i*2] = ' ';
        videoram[i*2+1] = 0x07;
    }
    xpos = ypos = 0;
}

void earlyprint(const char* s) {
    while(*s != '\0') {
        putc(*s);
        s++;
    }
}

void earlyprint(uint64 val) {
    char str[19];
    const char* hextable = "0123456789abcdef";
    str[0] = '0'; str[1] = 'x'; str[18] = '\0';
    for(int i = 0; i < 16; i++) {
        str[17-i] = hextable[(val >> (4*i)) & 0x0f];
    }
    earlyprint(str);
}

void earlyprint(uint32 val) {
    char str[11];
    const char* hextable = "0123456789abcdef";
    str[0] = '0'; str[1] = 'x'; str[10] = '\0';
    for(int i = 0; i < 8; i++) {
        str[9-i] = hextable[(val >> (4*i)) & 0x0f];
    }
    earlyprint(str);
}

void earlyprint(uint16 val) {
    char str[7];
    const char* hextable = "0123456789abcdef";
    str[0] = '0'; str[1] = 'x'; str[6] = '\0';
    for(int i = 0; i < 4; i++) {
        str[5-i] = hextable[(val >> (4*i)) & 0x0f];
    }
    earlyprint(str);
}