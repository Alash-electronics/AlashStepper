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


  // ===== Биполярный мотор через два H-моста (L298N, L293D, TB6612, DRV8833) =====
  {
    AlashStepper b(AlashStepper::DRIVER_BIPOLAR_4WIRE, 8, 9, 10, 11, 200);
    b.begin(AlashStepper::HALF_STEP); b.setAcceleration(0);
    CHECK(b.stepsPerRevolution() == 400 && b.driver() == AlashStepper::DRIVER_BIPOLAR_4WIRE);
    b.moveSteps(8); Trace tb = drive(b);
    const uint8_t biHalf[] = {0b0101, 0b0100, 0b0110, 0b0010, 0b1010, 0b1000, 0b1001, 0b0001};     // A+, A+B+, B+, A-B+, A-, A-B-, B-, A+B-
    CHECK(tb.masks.size() == 8);
    for (int i = 0; i < 8 && i < (int)tb.masks.size(); i++) CHECK(tb.masks[i] == biHalf[i]);
    AlashStepper f(AlashStepper::DRIVER_BIPOLAR_4WIRE, 8, 9, 10, 11, 400);
    f.begin(AlashStepper::FULL_STEP); f.setAcceleration(0); CHECK(f.stepsPerRevolution() == 400);
    f.moveSteps(4); tb = drive(f);
    const uint8_t biFull[] = {0b0110, 0b1010, 0b1001, 0b0101};
    CHECK(tb.masks.size() == 4);
    for (int i = 0; i < 4 && i < (int)tb.masks.size(); i++) CHECK(tb.masks[i] == biFull[i]);
    for (int i = 0; i < 4; i++) { int a = (biFull[i] & 1) + ((biFull[i] >> 1) & 1), c2 = ((biFull[i] >> 2) & 1) + ((biFull[i] >> 3) & 1); CHECK(a == 1 && c2 == 1); }   // в каждой обмотке ток только в одну сторону (нет «обоих плечей» сразу)
    f.setGearRatio(5.0f); CHECK(f.stepsPerRevolution() == 2000);                                   // редуктор 5:1
    f.moveSteps(-4); drive(f); CHECK(f.position() == 0);
  }

  // ===== Драйвер STEP/DIR (A4988, DRV8825, TMC2208, TB6600) =====
  {
    auto rises = [](int pin) { int n = 0; for (size_t i = 0; i < g_log.size(); i++) if (g_log[i].pin == pin && g_log[i].v == HIGH && (i == 0 || g_log[i - 1].pin != pin || g_log[i - 1].v != HIGH)) n++; return n; };
    g_log.clear(); g_logOn = true; g_us += 1000000;
    AlashStepper d(AlashStepper::DRIVER_STEP_DIR, 4, 5, 200);
    d.setEnablePin(6, true);                                      // EN активен LOW
    d.begin(); d.setAcceleration(0);
    CHECK(g_pin[6] == HIGH && d.stepsPerRevolution() == 200);     // драйвер выключен, пока мотор стоит
    CHECK(d.setMicrosteps(16) && d.stepsPerRevolution() == 3200 && d.microsteps() == 16);
    d.setSpeedSteps(5000);
    g_log.clear(); long w0 = g_writes; (void)w0;
    d.moveSteps(100); drive(d, 5);
    CHECK(rises(4) == 100 && d.position() == 100);
    bool dirHigh = false, enLow = false; for (auto &e : g_log) { if (e.pin == 5 && e.v == HIGH) dirHigh = true; if (e.pin == 6 && e.v == LOW) enLow = true; }
    CHECK(dirHigh && enLow);                                      // направление вперёд = DIR HIGH, драйвер включался на время движения
    bool ok = true; for (size_t i = 0; i + 1 < g_log.size(); i++) if (g_log[i].pin == 4 && g_log[i].v == HIGH && g_log[i + 1].pin == 4) ok = ok && (g_log[i + 1].t - g_log[i].t == 4);
    CHECK(ok);                                                    // импульс STEP по умолчанию 4 мкс
    g_log.clear(); d.moveSteps(-50); drive(d, 5);
    CHECK(rises(4) == 50 && d.position() == 50 && g_pin[5] == LOW);   // назад: DIR LOW
    g_us += 700000; d.run(); CHECK(g_pin[6] == HIGH);              // через 500 мс покоя драйвер выключен (EN неактивный)
    d.reverse(); g_log.clear(); d.moveSteps(10); drive(d, 5); CHECK(g_pin[5] == LOW && rises(4) == 10);   // reverse() меняет DIR
    d.reverse(false);
    d.setPulseWidthUs(10); g_log.clear(); d.moveSteps(3); drive(d, 5);
    ok = true; for (size_t i = 0; i + 1 < g_log.size(); i++) if (g_log[i].pin == 4 && g_log[i].v == HIGH && g_log[i + 1].pin == 4) ok = ok && (g_log[i + 1].t - g_log[i].t == 10);
    CHECK(ok);
    d.setPulseWidthUs(0); CHECK(d.error() == AlashStepper::ERR_BAD_VALUE); d.clearError();
    g_logOn = false;

    // скорость в шагах в секунду и в оборотах: 5000 шагов за секунду
    AlashStepper sp(AlashStepper::DRIVER_STEP_DIR, 4, 5, 200); sp.begin(); sp.setAcceleration(0); sp.setMicrosteps(16); sp.setSpeedSteps(5000);
    sp.moveSteps(5000); uint32_t t0 = g_us; drive(sp, 5); double sec = (g_us - t0) / 1e6; CHECK(sec > 0.95 && sec < 1.15);
    AlashStepper rp(AlashStepper::DRIVER_STEP_DIR, 4, 5, 200); rp.begin(); rp.setAcceleration(0); rp.setMicrosteps(16); rp.setSpeedRPM(60);   // 60 об/мин = 3200 шагов/с
    rp.moveSteps(3200); t0 = g_us; drive(rp, 5); sec = (g_us - t0) / 1e6; CHECK(sec > 0.95 && sec < 1.2);
    rp.setSpeedRPM(99999); rp.setSpeedSteps(-5); CHECK(rp.error() == AlashStepper::ERR_BAD_VALUE);
    AlashStepper ac(AlashStepper::DRIVER_STEP_DIR, 4, 5, 200); ac.begin(); ac.setMicrosteps(16); ac.setSpeedSteps(8000); ac.setAccelerationSteps(20000);
    ac.moveSteps(3000); Trace ta = drive(ac, 5);
    CHECK(ta.gaps.size() == 3000 && ta.gaps[3] > ta.gaps[1500] * 2 && ta.gaps[2995] > ta.gaps[1500] * 2);   // разгон и торможение
    AlashStepper hl(AlashStepper::DRIVER_STEP_DIR, 4, 5, 400); hl.setEnablePin(6, false); hl.begin();       // 0.9 градуса, EN активен HIGH
    CHECK(g_pin[6] == LOW && hl.stepsPerRevolution() == 400);
    hl.setAcceleration(0); hl.moveSteps(2); drive(hl, 5); CHECK(g_pin[6] == HIGH);
    // без вывода EN: работает так же
    AlashStepper ne(AlashStepper::DRIVER_STEP_DIR, 4, 5); ne.begin(); ne.setAcceleration(0); ne.moveSteps(5); drive(ne, 5); CHECK(ne.position() == 5);
  }

  // ===== Микрошаг пинами MS1..MS3: таблицы A4988 и DRV8825 =====
  {
    AlashStepper a(AlashStepper::DRIVER_STEP_DIR, 4, 5, 200); a.setMicrostepPins(1, 2, 3, AlashStepper::MS_A4988); a.begin();
    auto ms = [&]() { return g_pin[1] | (g_pin[2] << 1) | (g_pin[3] << 2); };
    CHECK(a.setMicrosteps(1) && ms() == 0b000 && a.setMicrosteps(2) && ms() == 0b001 && a.setMicrosteps(4) && ms() == 0b010);
    CHECK(a.setMicrosteps(8) && ms() == 0b011 && a.setMicrosteps(16) && ms() == 0b111 && a.stepsPerRevolution() == 3200);
    CHECK(!a.setMicrosteps(32) && a.error() == AlashStepper::ERR_NOT_SUPPORTED && a.microsteps() == 16 && ms() == 0b111);   // у A4988 нет 1/32, прежнее значение сохранено
    a.clearError(); CHECK(!a.setMicrosteps(3) && a.error() == AlashStepper::ERR_BAD_VALUE && !a.setMicrosteps(0) && !a.setMicrosteps(64));
    AlashStepper d(AlashStepper::DRIVER_STEP_DIR, 4, 5, 200); d.setMicrostepPins(1, 2, 3, AlashStepper::MS_DRV8825); d.begin();
    CHECK(d.setMicrosteps(16) && ms() == 0b100 && d.setMicrosteps(32) && ms() == 0b111 && d.stepsPerRevolution() == 6400);
    CHECK(d.setMicrosteps(8) && ms() == 0b011 && d.setMicrosteps(1) && ms() == 0b000);
    AlashStepper j(AlashStepper::DRIVER_STEP_DIR, 4, 5, 200); j.begin(); CHECK(j.setMicrosteps(32) && j.stepsPerRevolution() == 6400);   // микрошаг перемычками: пины не нужны
  }

  // ===== Тексты ошибок и диагностика для каждого типа драйвера =====
  {
    AlashStepper e(AlashStepper::DRIVER_STEP_DIR, 4, 5); e.begin();
    CHECK(strcmp(e.errorText(), "OK") == 0);
    e.setMicrosteps(3);
    e.setLanguage(AlashStepper::LANG_RU); std::string ru = e.errorText(); e.setLanguage(AlashStepper::LANG_KK); std::string kk = e.errorText(); e.setLanguage(AlashStepper::LANG_EN); std::string en = e.errorText();
    CHECK(ru.size() > 20 && kk.size() > 20 && en.size() > 20 && ru != kk && kk != en);
    g_autoTick = true;
    Print p; e.setEnablePin(6); e.diagnose(p);
    CHECK(p.text.find("STEP=4") != std::string::npos && p.text.find("DIR=5") != std::string::npos && p.text.find("EN=6") != std::string::npos && p.text.find("47 uF") != std::string::npos && p.text.find("Vref") != std::string::npos);
    AlashStepper bi(AlashStepper::DRIVER_BIPOLAR_4WIRE, 8, 9, 10, 11); bi.begin(); bi.setLanguage(AlashStepper::LANG_EN); Print pb; bi.diagnose(pb);
    CHECK(pb.text.find("two H-bridges") != std::string::npos && pb.text.find("ENA / ENB") != std::string::npos);
    AlashStepper u(8, 9, 10, 11); u.begin(); u.setLanguage(AlashStepper::LANG_EN); Print pu; u.diagnose(pu); CHECK(pu.text.find("ULN2003") != std::string::npos);
    g_autoTick = false;
  }

  z.setLanguage(AlashStepper::LANG_KK);
  g_autoTick = true;
  z.diagnose(*(new Print));

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
