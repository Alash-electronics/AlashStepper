// MIT License. Alash Electronics.
#include "AlashStepper.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

// Катушки в порядке IN1..IN4 (бит 0 = IN1). Через 8 фаз (или 4) картина повторяется.
static const uint8_t HALF_SEQ[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};
static const uint8_t FULL_SEQ[4] = {0b0011, 0b0110, 0b1100, 0b1001};

AlashStepper::AlashStepper(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4) {
  _p[0] = in1; _p[1] = in2; _p[2] = in3; _p[3] = in4;
}

void AlashStepper::begin(Mode mode) {
  _mode = mode;
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(_p[i], OUTPUT);
    digitalWrite(_p[i], LOW);
  }
  setGearRatio(_gear);
  _phase = 0;
  _energized = false;
}

void AlashStepper::setGearRatio(float ratio) {
  _gear = ratio;
  _stepsPerRev = 32.0f * ratio * (_mode == HALF_STEP ? 2.0f : 1.0f);   // у самого мотора 32 шага на оборот, дальше редуктор
  updateSpeeds();
}

void AlashStepper::updateSpeeds() {
  _maxSps = _rpm * _stepsPerRev / 60.0f;
  _accelSps2 = _accelRpmS * _stepsPerRev / 60.0f;
  _v0 = _maxSps < 100.0f ? _maxSps : 100.0f;            // с этой скорости мотор стартует без пропуска шагов
}

void AlashStepper::setSpeedRPM(float rpm) {
  if (rpm < 0.5f) rpm = 0.5f;
  if (rpm > 15.0f) rpm = 15.0f;                          // быстрее 28BYJ-48 пропускает шаги
  _rpm = rpm;
  updateSpeeds();
}

void AlashStepper::setAcceleration(float rpmPerSecond) {
  _accelRpmS = rpmPerSecond < 0 ? 0 : rpmPerSecond;
  updateSpeeds();
}

void AlashStepper::applyPhase() {
  uint8_t mask = _mode == HALF_STEP ? HALF_SEQ[_phase & 7] : FULL_SEQ[_phase & 3];
  for (uint8_t i = 0; i < 4; i++) digitalWrite(_p[i], (mask >> i) & 1 ? HIGH : LOW);
  _energized = true;
}

void AlashStepper::release() {
  for (uint8_t i = 0; i < 4; i++) digitalWrite(_p[i], LOW);
  _energized = false;
}

float AlashStepper::positionDegrees() const { return _pos * 360.0f / _stepsPerRev; }

// ---- Команды движения -------------------------------------------------------------

void AlashStepper::startMove(long newTarget) {
  run();
  if (newTarget == _pos) {
    _target = newTarget;
    return;
  }
  int8_t dir = newTarget > _pos ? 1 : -1;
  if (!_moving || dir != _dir || _continuous) _startPos = _pos;   // разгон начинается заново только при смене направления
  _dir = dir;
  _target = newTarget;
  _continuous = false;
  if (!_moving) {
    _moving = true;
    _intervalUs = (uint32_t)(1000000.0f / _v0);
    _lastStepAt = micros() - _intervalUs;                          // первый шаг сразу
  }
}

void AlashStepper::moveSteps(long steps) { startMove((_moving && !_continuous ? _target : _pos) + steps); }
void AlashStepper::moveDegrees(float degrees) { moveSteps((long)lroundf(degrees * _stepsPerRev / 360.0f)); }
void AlashStepper::rotate(float turns) { moveSteps((long)lroundf(turns * _stepsPerRev)); }
void AlashStepper::moveTo(float degrees) { startMove((long)lroundf(degrees * _stepsPerRev / 360.0f)); }

void AlashStepper::goToAngle(float degrees) {
  long rev = (long)lroundf(_stepsPerRev);
  long want = (long)lroundf(fmodf(fmodf(degrees, 360.0f) + 360.0f, 360.0f) * _stepsPerRev / 360.0f) % rev;
  long cur = ((_pos % rev) + rev) % rev;
  long d = want - cur;
  if (d > rev / 2) d -= rev;                                       // кратчайший путь
  if (d < -rev / 2) d += rev;
  startMove(_pos + d);
}

void AlashStepper::spin(bool forward) {
  run();
  int8_t dir = forward ? 1 : -1;
  if (!_moving || dir != _dir || !_continuous) _startPos = _pos;
  _dir = dir;
  _continuous = true;
  _target = _pos + dir * 2000000000L;                              // «бесконечно далеко»
  if (!_moving) {
    _moving = true;
    _intervalUs = (uint32_t)(1000000.0f / _v0);
    _lastStepAt = micros() - _intervalUs;
  }
}

void AlashStepper::stop() {
  if (!_moving) return;
  float v = 1000000.0f / (float)_intervalUs;
  long extra = _accelSps2 > 0 ? (long)((v * v - _v0 * _v0) / (2.0f * _accelSps2)) : 0;   // сколько шагов тормозить
  if (extra < 0) extra = 0;
  _target = _pos + _dir * (extra + 1);
  _continuous = false;
}

void AlashStepper::stopNow() {
  _moving = false;
  _continuous = false;
  _target = _pos;
  _idleSince = millis();
}

// Скорость в начале и конце движения растёт и падает по линейной рампе: v = sqrt(v0² + 2·a·путь)
uint32_t AlashStepper::intervalFor(long done, long remaining) {
  float v = _maxSps;
  if (_accelSps2 > 0) {
    long s = done < remaining ? done : remaining;
    float vr = sqrtf(_v0 * _v0 + 2.0f * _accelSps2 * (float)s);
    if (vr < v) v = vr;
  }
  if (v < 1.0f) v = 1.0f;
  return (uint32_t)(1000000.0f / v);
}

bool AlashStepper::run() {
  if (!_moving) {
    if (_energized && _releaseMs && (uint32_t)(millis() - _idleSince) >= _releaseMs) release();
    return false;
  }
  uint32_t now = micros();
  if ((uint32_t)(now - _lastStepAt) < _intervalUs) return true;
  _lastStepAt = now;

  int8_t d = _reversed ? -_dir : _dir;
  _phase = (uint8_t)(_phase + (d > 0 ? 1 : -1));
  applyPhase();
  _pos += _dir;

  long done = labs(_pos - _startPos), remaining = labs(_target - _pos);
  if (!_continuous && remaining == 0) {
    _moving = false;
    _idleSince = millis();
    return false;
  }
  _intervalUs = intervalFor(done, _continuous ? 2000000000L : remaining);
  return true;
}

void AlashStepper::wait() {
  while (run()) {}
}

// ---- Проверка и диагностика -----------------------------------------------------------

void AlashStepper::test() {
  moveDegrees(45);
  wait();
  moveDegrees(-45);
  wait();
}

void AlashStepper::diagnose(Print &out) {
  out.println(T("AlashStepper: диагностика", "AlashStepper: диагностика", "AlashStepper: diagnostics"));
  out.print(T("Пины IN1..IN4: ", "IN1..IN4 пиндері: ", "Pins IN1..IN4: "));
  for (uint8_t i = 0; i < 4; i++) {
    out.print((int)_p[i]);
    out.print(i < 3 ? F(", ") : F(""));
  }
  out.println();
  out.print(T("Режим: ", "Режим: ", "Mode: "));
  out.println(_mode == HALF_STEP ? T("полушаг (HALF_STEP)", "жартылай қадам (HALF_STEP)", "half-step (HALF_STEP)")
                                  : T("полный шаг (FULL_STEP)", "толық қадам (FULL_STEP)", "full-step (FULL_STEP)"));
  out.print(T("Шагов на оборот: ", "Бір айналымға қадам: ", "Steps per revolution: "));
  out.println((int)stepsPerRevolution());
  out.println(T("Сейчас мотор повернётся на 45 градусов вперёд и назад.", "Қазір мотор 45 градусқа алға және артқа бұрылады.", "The motor will now turn 45 degrees forward and back."));
  test();
  release();
  out.println(T("Если мотор дёргается и не вращается: IN1..IN4 подключены не по порядку; проверьте, что пины в конструкторе идут как IN1, IN2, IN3, IN4. Если не двигается совсем: питание платы ULN2003 должно быть 5 В (лучше отдельный источник, не пин 5V Arduino) и общий GND с Arduino. Если вращается в другую сторону: reverse(). Если мотор горячий: оставьте автоотключение катушек (setAutoRelease).",
                "Мотор тартылып, айналмаса: IN1..IN4 реті бойынша қосылмаған; конструктордағы пиндер IN1, IN2, IN3, IN4 ретімен екенін тексеріңіз. Мүлде қозғалмаса: ULN2003 тақтасының қуаты 5 В болуы керек (бөлек көз жақсы, Arduino 5V пині емес) және Arduino-мен ортақ GND. Басқа жаққа айналса: reverse(). Мотор ыстық болса: катушкаларды автоөшіруді қалдырыңыз (setAutoRelease).",
                "If the motor jitters but does not turn: IN1..IN4 are connected out of order; make sure the constructor pins go as IN1, IN2, IN3, IN4. If it does not move at all: the ULN2003 board needs 5 V (preferably a separate supply, not the Arduino 5V pin) and a common GND with the Arduino. If it turns the other way: reverse(). If the motor is hot: keep the coil auto-release (setAutoRelease)."));
}
