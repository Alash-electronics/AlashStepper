// MIT License. Alash Electronics.
#include "AlashStepper.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

// Катушки в порядке IN1..IN4 (бит 0 = IN1). Через 8 фаз (или 4) картина повторяется.
static const uint8_t HALF_SEQ[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};
static const uint8_t FULL_SEQ[4] = {0b0011, 0b0110, 0b1100, 0b1001};
// Биполярный мотор через два H-моста, пины a1, a2, b1, b2 (бит 0 = a1): ток обмотки A: a1 или a2, обмотки B: b1 или b2
static const uint8_t BI_HALF_SEQ[8] = {0b0001, 0b0101, 0b0100, 0b0110, 0b0010, 0b1010, 0b1000, 0b1001};
static const uint8_t BI_FULL_SEQ[4] = {0b0101, 0b0110, 0b1010, 0b1001};
// Микрошаг: значение для пинов MS1, MS2, MS3 (бит 0 = MS1) для 1, 2, 4, 8, 16, 32 микрошагов (значение 0xFF = не поддерживается)
static const uint8_t MS_TAB_A4988[6] = {0b000, 0b001, 0b010, 0b011, 0b111, 0xFF};
static const uint8_t MS_TAB_DRV8825[6] = {0b000, 0b001, 0b010, 0b011, 0b100, 0b111};

AlashStepper::AlashStepper(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4) {
  _p[0] = in1; _p[1] = in2; _p[2] = in3; _p[3] = in4;
}

AlashStepper::AlashStepper(Driver driver, uint8_t a1, uint8_t a2, uint8_t b1, uint8_t b2, uint16_t fullStepsPerRev) {
  _p[0] = a1; _p[1] = a2; _p[2] = b1; _p[3] = b2;
  _driver = driver == DRIVER_ULN2003 ? DRIVER_ULN2003 : DRIVER_BIPOLAR_4WIRE;
  _fullSteps = fullStepsPerRev ? fullStepsPerRev : 200;
  if (_driver != DRIVER_ULN2003) { _gear = 1.0f; _rpm = 60; _accelRpmS = 120; }
}

AlashStepper::AlashStepper(Driver driver, uint8_t stepPin, uint8_t dirPin, uint16_t fullStepsPerRev) {
  _p[0] = stepPin; _p[1] = dirPin; _p[2] = _p[3] = 255;
  _driver = DRIVER_STEP_DIR; (void)driver;
  _fullSteps = fullStepsPerRev ? fullStepsPerRev : 200;
  _gear = 1.0f; _rpm = 60; _accelRpmS = 120;
}

void AlashStepper::begin(Mode mode) {
  _mode = mode;
  uint8_t n = _driver == DRIVER_STEP_DIR ? 2 : 4;
  for (uint8_t i = 0; i < n; i++) {
    pinMode(_p[i], OUTPUT);
    digitalWrite(_p[i], LOW);
  }
  if (_en != 255) { pinMode(_en, OUTPUT); digitalWrite(_en, _enLow ? HIGH : LOW); }      // драйвер выключен, пока не двигаем
  if (_ms[0] != 255) setMicrosteps(_micro);                                               // выставить MS1..MS3, если заданы
  setGearRatio(_gear);
  _phase = 0;
  _energized = false;
}

// Шагов на оборот выхода: у 28BYJ-48 32 шага мотора на оборот и редуктор; у остальных полные шаги умножаются на микрошаги и редуктор
void AlashStepper::setGearRatio(float ratio) {
  _gear = ratio;
  if (_driver == DRIVER_ULN2003) _stepsPerRev = 32.0f * ratio * (_mode == HALF_STEP ? 2.0f : 1.0f);
  else if (_driver == DRIVER_BIPOLAR_4WIRE) _stepsPerRev = (float)_fullSteps * ratio * (_mode == HALF_STEP ? 2.0f : 1.0f);
  else _stepsPerRev = (float)_fullSteps * _micro * ratio;
  updateSpeeds();
}

void AlashStepper::updateSpeeds() {
  if (_spsMode) _maxSps = _spsSet; else _maxSps = _rpm * _stepsPerRev / 60.0f;
  _accelSps2 = _accelSet2 >= 0 ? _accelSet2 : _accelRpmS * _stepsPerRev / 60.0f;
  float startMax = _driver == DRIVER_ULN2003 ? 100.0f : 200.0f;
  _v0 = _maxSps < startMax ? _maxSps : startMax;         // с этой скорости мотор стартует без пропуска шагов
}

void AlashStepper::setSpeedRPM(float rpm) {
  if (rpm < 0.5f) rpm = 0.5f;
  float limit = _driver == DRIVER_ULN2003 ? 15.0f : 3000.0f;   // быстрее 28BYJ-48 пропускает шаги; остальные ограничены частотой шагов
  if (rpm > limit) rpm = limit;
  _rpm = rpm; _spsMode = false;
  updateSpeeds();
}

void AlashStepper::setSpeedSteps(float sps) {
  if (!(sps >= 1.0f)) { _err = ERR_BAD_VALUE; return; }
  if (sps > 50000.0f) sps = 50000.0f;                    // выше библиотека на обычной плате шаги не успеет делать
  _spsSet = sps; _spsMode = true;
  updateSpeeds();
}

void AlashStepper::setAccelerationSteps(float a) {
  if (!(a >= 0.0f)) { _err = ERR_BAD_VALUE; return; }
  _accelSet2 = a;
  updateSpeeds();
}

void AlashStepper::setEnablePin(uint8_t pin, bool activeLow) {
  _en = pin; _enLow = activeLow;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, activeLow ? HIGH : LOW);
}

void AlashStepper::setPulseWidthUs(uint16_t us) {
  if (us < 1 || us > 1000) { _err = ERR_BAD_VALUE; return; }
  _pulseUs = us;
}

void AlashStepper::setMicrostepPins(uint8_t ms1, uint8_t ms2, uint8_t ms3, MicrostepChip chip) {
  _ms[0] = ms1; _ms[1] = ms2; _ms[2] = ms3; _msChip = chip;
  for (uint8_t i = 0; i < 3; i++) if (_ms[i] != 255) pinMode(_ms[i], OUTPUT);
}

bool AlashStepper::setMicrosteps(uint16_t n) {
  uint8_t idx = 255;
  for (uint8_t i = 0; i < 6; i++) if (n == (1u << i)) idx = i;
  if (idx == 255) { _err = ERR_BAD_VALUE; return false; }
  if (_ms[0] != 255 || _ms[1] != 255 || _ms[2] != 255) {
    uint8_t v = _msChip == MS_A4988 ? MS_TAB_A4988[idx] : MS_TAB_DRV8825[idx];
    if (v == 0xFF) { _err = ERR_NOT_SUPPORTED; return false; }       // у A4988 максимум 16 микрошагов
    for (uint8_t i = 0; i < 3; i++) if (_ms[i] != 255) digitalWrite(_ms[i], (v >> i) & 1 ? HIGH : LOW);
  }
  _micro = n;
  setGearRatio(_gear);
  return true;
}

void AlashStepper::setAcceleration(float rpmPerSecond) {
  _accelRpmS = rpmPerSecond < 0 ? 0 : rpmPerSecond;
  updateSpeeds();
}

void AlashStepper::applyPhase() {
  if (_driver == DRIVER_STEP_DIR) {                       // направление задаёт _phase (знак последнего шага), здесь один импульс STEP
    if (!_energized && _en != 255) { digitalWrite(_en, _enLow ? LOW : HIGH); delayMicroseconds(10); }
    digitalWrite(_p[1], _dirLevel ? HIGH : LOW);
    delayMicroseconds(2);                                 // драйверу нужно время после смены DIR
    digitalWrite(_p[0], HIGH);
    delayMicroseconds(_pulseUs);
    digitalWrite(_p[0], LOW);
    _energized = true;
    return;
  }
  uint8_t mask;
  if (_driver == DRIVER_BIPOLAR_4WIRE) mask = _mode == HALF_STEP ? BI_HALF_SEQ[_phase & 7] : BI_FULL_SEQ[_phase & 3];
  else mask = _mode == HALF_STEP ? HALF_SEQ[_phase & 7] : FULL_SEQ[_phase & 3];
  for (uint8_t i = 0; i < 4; i++) digitalWrite(_p[i], (mask >> i) & 1 ? HIGH : LOW);
  _energized = true;
}

void AlashStepper::release() {
  if (_driver == DRIVER_STEP_DIR) { if (_en != 255) digitalWrite(_en, _enLow ? HIGH : LOW); }
  else for (uint8_t i = 0; i < 4; i++) digitalWrite(_p[i], LOW);
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
  _dirLevel = d > 0;                                       // для STEP/DIR: уровень на DIR
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

const __FlashStringHelper *AlashStepper::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_BAD_VALUE:
      return T("Значение вне допустимого: микрошаг 1, 2, 4, 8, 16 или 32; скорость, разгон и длина импульса положительные числа.",
               "Мән жарамсыз: микроқадам 1, 2, 4, 8, 16 немесе 32; жылдамдық, үдеу және импульс ұзындығы оң сандар болуы керек.",
               "The value is out of range: the microstep is 1, 2, 4, 8, 16 or 32; the speed, acceleration and pulse width are positive numbers.");
    case ERR_NOT_SUPPORTED:
      return T("Этот микрошаг не поддерживается выбранной микросхемой: у A4988 максимум 16, у DRV8825 до 32.",
               "Бұл микроқадамды таңдалған микросхема қолдамайды: A4988-де ең көбі 16, DRV8825-те 32-ге дейін.",
               "This microstep is not supported by the chosen chip: the A4988 goes up to 16, the DRV8825 up to 32.");
  }
  return F("");
}

void AlashStepper::diagnose(Print &out) {
  out.println(T("AlashStepper: диагностика", "AlashStepper: диагностика", "AlashStepper: diagnostics"));
  if (_driver == DRIVER_STEP_DIR) {
    out.print(T("Драйвер: STEP/DIR   STEP=", "Драйвер: STEP/DIR   STEP=", "Driver: STEP/DIR   STEP=")); out.print((int)_p[0]);
    out.print(F("  DIR=")); out.print((int)_p[1]);
    if (_en != 255) { out.print(F("  EN=")); out.print((int)_en); out.print(_enLow ? F(" (active LOW)") : F(" (active HIGH)")); }
    out.println();
    out.print(T("Полных шагов на оборот: ", "Бір айналымға толық қадам: ", "Full steps per revolution: ")); out.print((int)_fullSteps);
    out.print(T("   микрошаг: 1/", "   микроқадам: 1/", "   microstep: 1/")); out.print((int)_micro);
    out.print(T("   импульс, мкс: ", "   импульс, мкс: ", "   pulse, us: ")); out.println((int)_pulseUs);
  } else {
    out.print(_driver == DRIVER_BIPOLAR_4WIRE ? T("Драйвер: два H-моста, пины A1, A2, B1, B2: ", "Драйвер: екі H-көпір, A1, A2, B1, B2 пиндері: ", "Driver: two H-bridges, pins A1, A2, B1, B2: ")
                                               : T("Пины IN1..IN4: ", "IN1..IN4 пиндері: ", "Pins IN1..IN4: "));
    for (uint8_t i = 0; i < 4; i++) {
      out.print((int)_p[i]);
      out.print(i < 3 ? F(", ") : F(""));
    }
    out.println();
    out.print(T("Режим: ", "Режим: ", "Mode: "));
    out.println(_mode == HALF_STEP ? T("полушаг (HALF_STEP)", "жартылай қадам (HALF_STEP)", "half-step (HALF_STEP)")
                                    : T("полный шаг (FULL_STEP)", "толық қадам (FULL_STEP)", "full-step (FULL_STEP)"));
  }
  out.print(T("Шагов на оборот: ", "Бір айналымға қадам: ", "Steps per revolution: "));
  out.println((int)stepsPerRevolution());
  out.println(T("Сейчас мотор повернётся на 45 градусов вперёд и назад.", "Қазір мотор 45 градусқа алға және артқа бұрылады.", "The motor will now turn 45 degrees forward and back."));
  test();
  release();
  if (_driver == DRIVER_STEP_DIR) {
    out.println(T("Подсказки для A4988 / DRV8825 / TB6600: питание мотора 8-35 В на VMOT и GND, логика 3.3-5 В, между VMOT и GND конденсатор не менее 47 мкФ (без него драйвер может сгореть). Никогда не отключайте мотор от драйвера при включённом питании. Выход EN включается уровнем LOW: укажите setEnablePin() или соедините EN с GND. Выводы RESET и SLEEP драйвера должны быть соединены (или подтянуты к 5 В). Не вращается и греется: поверните подстроечный резистор тока (Vref), проверьте порядок обмоток мотора (две пары проводов: измерьте сопротивление). Дёргается: уменьшите скорость или увеличьте setAcceleration. Крутится не в ту сторону: reverse() или поменяйте пару проводов. Шагов на оборот не сходятся с реальными: проверьте перемычки микрошага и setMicrosteps().",
                  "A4988 / DRV8825 / TB6600 кеңестері: мотор қуаты VMOT және GND-ға 8-35 В, логика 3.3-5 В, VMOT пен GND арасында кемінде 47 мкФ конденсатор (онсыз драйвер жанып кетуі мүмкін). Қуат қосулы кезде моторды драйверден ешқашан ажыратпаңыз. EN шығысы LOW деңгейімен қосылады: setEnablePin() көрсетіңіз немесе EN-ді GND-ға жалғаңыз. RESET және SLEEP шықпалары жалғануы керек. Айналмай ыстық болса: ток реттегішін (Vref) бұраңыз, мотор орамдарының ретін тексеріңіз. Тартылса: жылдамдықты азайтыңыз немесе setAcceleration көбейтіңіз. Кері айналса: reverse() немесе сым жұбын ауыстырыңыз.",
                  "Tips for A4988 / DRV8825 / TB6600: motor power 8-35 V on VMOT and GND, logic 3.3-5 V, a capacitor of at least 47 uF between VMOT and GND (without it the driver can burn). Never disconnect the motor from the driver while the power is on. EN is active LOW: give setEnablePin() or join EN to GND. The RESET and SLEEP pins of the driver must be joined. If it does not turn and gets hot: turn the current trimmer (Vref), check the motor winding order (two wire pairs: measure the resistance). If it jitters: lower the speed or raise setAcceleration. If it turns the wrong way: reverse() or swap a wire pair."));
  } else if (_driver == DRIVER_BIPOLAR_4WIRE) {
    out.println(T("Подсказки для L298N / L293D / TB6612: пины A1, A2 управляют одной обмоткой, B1, B2 другой; если мотор дёргается, а не вращается, поменяйте местами провода одной обмотки или порядок пинов. На L298N снимите перемычки ENA / ENB или подайте на них HIGH. Питание мотора отдельно от Arduino, общий GND. Мотор горячий: оставьте автоотключение катушек (setAutoRelease).",
                  "L298N / L293D / TB6612 кеңестері: A1, A2 пиндері бір орамды, B1, B2 екіншісін басқарады; мотор айналмай тартылса, бір орам сымдарын немесе пиндер ретін ауыстырыңыз. L298N-де ENA / ENB перемычкаларын алыңыз немесе оларға HIGH беріңіз. Мотор қуаты Arduino-дан бөлек, GND ортақ. Мотор ыстық болса: катушкаларды автоөшіруді қалдырыңыз (setAutoRelease).",
                  "Tips for L298N / L293D / TB6612: pins A1, A2 drive one winding, B1, B2 the other; if the motor jitters instead of turning, swap the wires of one winding or the pin order. On the L298N remove the ENA / ENB jumpers or give them HIGH. Power the motor separately from the Arduino with a common GND. If the motor is hot: keep the coil auto-release (setAutoRelease)."));
  } else {
    out.println(T("Если мотор дёргается и не вращается: IN1..IN4 подключены не по порядку; проверьте, что пины в конструкторе идут как IN1, IN2, IN3, IN4. Если не двигается совсем: питание платы ULN2003 должно быть 5 В (лучше отдельный источник, не пин 5V Arduino) и общий GND с Arduino. Если вращается в другую сторону: reverse(). Если мотор горячий: оставьте автоотключение катушек (setAutoRelease).",
                  "Мотор тартылып, айналмаса: IN1..IN4 реті бойынша қосылмаған; конструктордағы пиндер IN1, IN2, IN3, IN4 ретімен екенін тексеріңіз. Мүлде қозғалмаса: ULN2003 тақтасының қуаты 5 В болуы керек (бөлек көз жақсы, Arduino 5V пині емес) және Arduino-мен ортақ GND. Басқа жаққа айналса: reverse(). Мотор ыстық болса: катушкаларды автоөшіруді қалдырыңыз (setAutoRelease).",
                  "If the motor jitters but does not turn: IN1..IN4 are connected out of order; make sure the constructor pins go as IN1, IN2, IN3, IN4. If it does not move at all: the ULN2003 board needs 5 V (preferably a separate supply, not the Arduino 5V pin) and a common GND with the Arduino. If it turns the other way: reverse(). If the motor is hot: keep the coil auto-release (setAutoRelease)."));
  }
}
