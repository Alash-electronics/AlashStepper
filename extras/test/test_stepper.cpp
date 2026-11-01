// Тесты AlashStepper без железа (время и катушки имитирует тест):
//   g++ -std=c++17 -Wall -I mock -I ../../src test_stepper.cpp ../../src/AlashStepper.cpp && ./a.out
#include <stdio.h>
#include <vector>
#include "Arduino.h"
#include "AlashStepper.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)

static uint8_t mask() { return g_pin[8] | (g_pin[9] << 1) | (g_pin[10] << 2) | (g_pin[11] << 3); }

// Крутим время, пока мотор движется; запоминаем маску катушек и интервалы между шагами
struct Trace { std::vector<uint8_t> masks; std::vector<uint32_t> gaps; };
static Trace drive(AlashStepper &s, uint32_t tick = 20, uint32_t limitUs = 200000000u) {
  Trace t; uint32_t last = g_us; long w = g_writes; uint32_t start = g_us;
  while (g_us - start < limitUs) {
    g_us += tick;
    bool mv = s.run();
    if (g_writes != w) { w = g_writes; t.masks.push_back(mask()); t.gaps.push_back(g_us - last); last = g_us; }
    if (!mv) break;
  }
  return t;
}

int main() {
  g_us = 1000000;
  AlashStepper s(8, 9, 10, 11);
  s.begin();
  s.setAcceleration(0);
  s.moveSteps(8);
  Trace t = drive(s);
  const uint8_t half[] = {0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001, 0b0001};   // первый шаг сразу переводит фазу 0 -> 1
  CHECK(t.masks.size() == 8);
  for (int i = 0; i < 8 && i < (int)t.masks.size(); i++) CHECK(t.masks[i] == half[i]);
  CHECK(s.position() == 8);

  s.moveSteps(-4);
  t = drive(s);
  CHECK(t.masks.size() == 4 && t.masks[0] == 0b1001 && t.masks[1] == 0b1000);
  CHECK(s.position() == 4);

  AlashStepper f(8, 9, 10, 11);
  f.begin(AlashStepper::FULL_STEP); f.setAcceleration(0);
  f.moveSteps(4);
  t = drive(f);
  const uint8_t full[] = {0b0110, 0b1100, 0b1001, 0b0011};
  CHECK(t.masks.size() == 4);
  for (int i = 0; i < 4 && i < (int)t.masks.size(); i++) CHECK(t.masks[i] == full[i]);
  CHECK(f.stepsPerRevolution() == 2038);

  AlashStepper a(8, 9, 10, 11);
  a.begin(); a.setAcceleration(0); a.setSpeedRPM(15);
  CHECK(a.stepsPerRevolution() == 4076);
  a.moveDegrees(360); drive(a);
  CHECK(a.position() == 4076 && fabsf(a.positionDegrees() - 360.0f) < 0.05f);
  a.moveDegrees(-90); drive(a);
  CHECK(a.position() == 4076 - 1019 && fabsf(a.positionDegrees() - 270.0f) < 0.1f);
  a.moveTo(0); drive(a);
  CHECK(a.position() == 0);
  a.rotate(0.5f); drive(a);
  CHECK(a.position() == 2038);
  a.setGearRatio(64.0f);
  CHECK(a.stepsPerRevolution() == 4096);

  AlashStepper g(8, 9, 10, 11);
  g.begin(); g.setAcceleration(0); g.setSpeedRPM(15);
  g.goToAngle(10); drive(g);
  long p10 = g.position();
  g.goToAngle(350); drive(g);
  CHECK(g.position() < p10 && fabsf(g.positionDegrees() - (-10.0f)) < 0.2f);
  g.goToAngle(-10); drive(g);                    // это тот же угол, двигаться не надо
  CHECK(fabsf(g.positionDegrees() - (-10.0f)) < 0.2f);

  AlashStepper v(8, 9, 10, 11);
  v.begin(); v.setAcceleration(0); v.setSpeedRPM(10);
  v.moveSteps(679);
  uint32_t t0 = g_us;
  drive(v);
  double sec = (g_us - t0) / 1e6;
  CHECK(sec > 0.95 && sec < 1.1);
  v.setSpeedRPM(100);                             // выше предела: ограничивается 15
  v.moveSteps(1019);
  t0 = g_us; drive(v);
  sec = (g_us - t0) / 1e6;
  CHECK(sec > 0.95 && sec < 1.1);                // 15 об/мин = 1019 шагов в секунду

  AlashStepper r(8, 9, 10, 11);
  r.begin(); r.setSpeedRPM(10); r.setAcceleration(15);
  r.moveSteps(2000);
  t = drive(r);
  CHECK(t.masks.size() == 2000);
  CHECK(t.gaps[5] > t.gaps[1000] * 2);            // старт заметно медленнее
  CHECK(t.gaps[1995] > t.gaps[1000] * 2);        // и торможение в конце
  CHECK(r.position() == 2000);

  AlashStepper c(8, 9, 10, 11);
  c.begin(); c.setSpeedRPM(10); c.setAcceleration(15);
  c.spin(true);
  uint32_t limit = g_us + 2000000;                // 2 секунды вращения
  while (g_us < limit) { g_us += 20; c.run(); }
  CHECK(c.isMoving());
  long before = c.position();
  CHECK(before > 800);
  c.stop();
  drive(c);
  CHECK(!c.isMoving() && c.position() > before && c.position() < before + 300);
  c.spin(false);
  for (int i = 0; i < 100; i++) { g_us += 20000; c.run(); }
  long mid = c.position();
  c.stopNow();
  g_us += 100000; c.run();
  CHECK(c.position() == mid && !c.isMoving());

  AlashStepper rv(8, 9, 10, 11);
  rv.begin(); rv.setAcceleration(0); rv.reverse();
  rv.moveSteps(2);
  t = drive(rv);
  CHECK(t.masks[0] == 0b1001);                    // первый шаг назад по последовательности (вместо 0b0011)
  CHECK(rv.position() == 2);                      // положение растёт так же

  AlashStepper h(8, 9, 10, 11);
  h.begin(); h.setAcceleration(0);
  h.moveSteps(3); drive(h);
  CHECK(mask() != 0);
  g_us += 300000; h.run(); CHECK(mask() != 0);    // 300 мс: ещё держит
  g_us += 400000; h.run(); CHECK(mask() == 0);    // 700 мс: отпустил
  h.setAutoRelease(0);
  h.moveSteps(3); drive(h);
  g_us += 5000000; h.run(); CHECK(mask() != 0);   // 0 = держать всегда

  AlashStepper n(8, 9, 10, 11);
  n.begin(); n.setSpeedRPM(10); n.setAcceleration(15);
  n.moveSteps(1000);
  for (int i = 0; i < 20000; i++) { g_us += 20; n.run(); }
  long here = n.position();
  n.moveSteps(500);                                 // добавить к цели
  drive(n);
  CHECK(n.position() == 1500 && here > 0);

  AlashStepper z(8, 9, 10, 11);
  z.begin(); z.setAcceleration(0);
  z.moveSteps(100);
  while (z.run()) g_us += 1000;
  z.setZero();
  CHECK(z.position() == 0);

  z.setLanguage(AlashStepper::LANG_KK);
  g_autoTick = true;
  z.diagnose(*(new Print));

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
