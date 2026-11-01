// Минимальная имитация Arduino.h для запуска тестов на компьютере (g++).
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using __FlashStringHelper = char;
#define F(x) ((const __FlashStringHelper *)(x))
#define OUTPUT 1
#define LOW 0
#define HIGH 1
inline uint32_t g_us = 0;                       // время, мкс
inline uint8_t g_pin[64];
inline long g_writes = 0;
inline bool g_autoTick = false;                // для блокирующих wait(): время идёт само
inline uint32_t micros() { if (g_autoTick) g_us += 50; return g_us; }
inline uint32_t millis() { return g_us / 1000; }
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t p, uint8_t v) { g_pin[p] = v; g_writes++; }
class Print {
 public:
  void print(const char *s) { printf("%s", s); }
  void print(int v) { printf("%d", v); }
  void println(const char *s = "") { printf("%s\n", s); }
  void println(int v) { printf("%d\n", v); }
};
