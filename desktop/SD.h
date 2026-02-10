#pragma once
#include "Arduino.h"

#define FILE_WRITE 1
#define FILE_READ 0

class File {
public:
    bool isDirectory(){ return false; }
    const char* name(){ return "stub.txt"; }
    uint32_t size(){ return 0; }
    bool available(){ return false; }
    String readStringUntil(char){ return ""; }
    void print(...) {}
    void println(...) {}
    void seek(int) {}
    void close() {}
};

class SDClass {
public:
    bool begin(uint8_t){ return true; }
    File open(const char*, int mode=0){ return File(); }
    bool exists(const char*){ return false; }
};

static SDClass SD;
