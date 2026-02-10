#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <cmath>
#include <algorithm>

// ---------------- PROGMEM EMULATION ----------------
#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char*)(addr))
#define pgm_read_ptr(addr) (*(const void**)(addr))

// ---------------- Arduino String ----------------
class String : public std::string {
public:
    using std::string::string;
    String(const std::string &s) : std::string(s) {}
    String() : std::string() {}
    String(const char* s) : std::string(s) {}

    int indexOf(const char* s){
        auto pos = find(s);
        return pos==npos ? -1 : (int)pos;
    }

    float toFloat(){
        return atof(c_str());
    }

    void trim(){
        size_t first = find_first_not_of(" \n\r\t");
        if(first==npos) { clear(); return; }
        size_t last = find_last_not_of(" \n\r\t");
        erase(last+1);
        erase(0, first);
    }

    void replace(const char* from,const char* to){
        size_t pos = find(from);
        if(pos!=npos)
            std::string::replace(pos, strlen(from), to);
    }

    String substring(size_t start,size_t end=npos){
        if(end==npos) return String(std::string::substr(start));
        return String(std::string::substr(start,end-start));
    }
};

// ---------------- Serial Mock ----------------
class SerialMock {
public:
    void begin(int){}

    template<typename T>
    void print(T v){ printf("%s", std::to_string(v).c_str()); }

    void print(const char* s){ printf("%s",s); }

    template<typename T>
    void println(T v){ printf("%s\n", std::to_string(v).c_str()); }

    void println(const char* s){ printf("%s\n",s); }
};

static SerialMock SerialUSB;
static SerialMock Serial1;

// ---------------- Arduino Helpers ----------------
#define F(x) x

inline long random(long a,long b){ return a + rand()%(b-a); }

inline int16_t map(int16_t x,int16_t in_min,int16_t in_max,int16_t out_min,int16_t out_max){
    return (int16_t)(((int32_t)(x-in_min)*(out_max-out_min))/(in_max-in_min)+out_min);
}

inline int16_t constrain(int16_t x,int16_t a,int16_t b){
    if(x < a) return a;
    if(x > b) return b;
    return x;
}

inline void delay(int){}
inline unsigned long millis(){ static unsigned long t=0; return t+=16; }

#define HIGH 1
#define LOW 0
inline void pinMode(int,int){}
inline void digitalWrite(int,int){}

#ifndef max
#define max(a,b) std::max(a,b)
#endif
