#pragma once

// Standard C/C++ headers
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// C++ STL headers
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>

// ---------------- PROGMEM EMULATION ----------------
#define PROGMEM
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#define pgm_read_ptr(addr) (*(const void**)(addr))

// ---------------- Arduino String ----------------
class String : public std::string {
public:
    using std::string::string;
    String(const std::string &s) : std::string(s) {}
    String() : std::string() {}
    String(const char* s) : std::string(s ? s : "") {}
    String(int v) : std::string(std::to_string(v)) {}
    String(float v) : std::string(std::to_string(v)) {}

    int indexOf(const char* s){
        auto pos = find(s);
        return pos==npos ? -1 : (int)pos;
    }

    int indexOf(char c){
        auto pos = find(c);
        return pos==npos ? -1 : (int)pos;
    }

    float toFloat(){
        return (float)atof(c_str());
    }

    void trim(){
        // Simple trim implementation
        if (length() == 0) return;
        size_t first = find_first_not_of(" \n\r\t");
        if (first == std::string::npos) { clear(); return; }
        size_t last = find_last_not_of(" \n\r\t");
        *this = substr(first, (last - first + 1));
    }

    void replace(String from, String to) {
        size_t start_pos = 0;
        while((start_pos = find(from, start_pos)) != std::string::npos) {
            std::string::replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    String substring(size_t start, size_t end = std::string::npos) {
        if(start >= length()) return String("");
        if(end == std::string::npos) return String(substr(start));
        return String(substr(start, end - start));
    }
};

// ---------------- Serial Mock ----------------
class SerialMock {
public:
    void begin(int){}
    
    template<typename T>
    void print(T v){ std::cout << v; }
    
    template<typename T>
    void println(T v){ std::cout << v << std::endl; }
    
    void println(){ std::cout << std::endl; }
};

static SerialMock SerialUSB;
static SerialMock Serial1;

// ---------------- Arduino Helpers ----------------
#define F(x) x

inline long random(long min, long max){ return min + (rand() % (max - min)); }

inline int16_t map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline int16_t constrain(int16_t x, int16_t a, int16_t b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

inline void delay(int ms){}
inline unsigned long millis(){ static unsigned long t=0; return t+=16; }

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

inline void pinMode(int, int){}
inline void digitalWrite(int, int){}
inline int digitalRead(int){ return 0; }

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif