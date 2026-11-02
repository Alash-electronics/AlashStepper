// AlashStepper: шаговые двигатели с любым драйвером: 28BYJ-48 + ULN2003, биполярные NEMA17 и другие через L298N / L293D / TB6612 (4 провода)
// и через драйверы STEP/DIR (A4988, DRV8825, TMC2208, TB6600): углы, обороты, микрошаги, плавный разгон, без блокировки.
// Без зависимостей. MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashStepper
#ifndef ALASH_STEPPER_H
#define ALASH_STEPPER_H

#include <Arduino.h>

class AlashStepper {
 public:
  // HALF_STEP: 8 фаз, плавнее и точнее (по умолчанию, ~4076 шагов на оборот). FULL_STEP: 4 фазы, сильнее (~2038).
  enum Mode : uint8_t { HALF_STEP = 0, FULL_STEP = 1 };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };
  // Тип драйвера: ULN2003 (28BYJ-48), BIPOLAR_4WIRE (две H-моста: L298N, L293D, TB6612, DRV8833; мотор с 4 проводами), STEP_DIR (A4988, DRV8825, TMC2208, TB6600 и подобные)
  enum Driver : uint8_t { DRIVER_ULN2003 = 0, DRIVER_BIPOLAR_4WIRE, DRIVER_STEP_DIR };
  // Микросхема драйвера для выбора микрошага пинами MS1..MS3 (у них разные таблицы)
  enum MicrostepChip : uint8_t { MS_A4988 = 0, MS_DRV8825 };
  enum Error : uint8_t { ERR_OK = 0, ERR_BAD_VALUE, ERR_NOT_SUPPORTED };

  // ULN2003 + 28BYJ-48. in1..in4: пины Arduino, подключённые к входам IN1..IN4 платы ULN2003 в том же порядке (не перепутывайте, как в старых примерах Stepper).
  AlashStepper(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4);
  // Биполярный мотор через два H-моста (4 пина управления): a1, a2 одна обмотка, b1, b2 другая. fullStepsPerRev: 200 (1.8 градуса) или 400 (0.9 градуса).
  AlashStepper(Driver driver, uint8_t a1, uint8_t a2, uint8_t b1, uint8_t b2, uint16_t fullStepsPerRev = 200);
  // Драйвер STEP/DIR: пины STEP и DIR. fullStepsPerRev: 200 (1.8 градуса) или 400 (0.9 градуса); микрошаги задайте setMicrosteps().
  AlashStepper(Driver driver, uint8_t stepPin, uint8_t dirPin, uint16_t fullStepsPerRev = 200);

  void begin(Mode mode = HALF_STEP);       // Mode для ULN2003 и BIPOLAR_4WIRE; у STEP/DIR шаг задают микрошаги

  // ---- Движение (не блокирует: вызывайте run() в loop(), или wait() для простых скетчей) ----------
  void moveDegrees(float degrees);         // повернуть на угол от текущего положения (минус = обратно)
  void moveTo(float degrees);              // повернуть в положение (углы от нуля, можно больше 360)
  void goToAngle(float degrees);           // повернуть на угол 0..360 кратчайшим путём
  void rotate(float turns);                // повернуть на число оборотов (0.5 = пол-оборота)
  void moveSteps(long steps);              // повернуть на число шагов
  void spin(bool forward = true);          // вращаться без остановки, пока не вызовете stop()
  void stop();                             // плавно остановиться
  void stopNow();                          // остановиться сразу
  bool run();                              // сделать шаг, если пора. true, пока мотор движется
  void wait();                             // дождаться конца движения (блокирует)
  bool isMoving() { run(); return _moving; }

  // ---- Положение ---------------------------------------------------------------
  long position() const { return _pos; }                  // в шагах
  float positionDegrees() const;                          // в градусах от нуля
  void setZero() { _pos = 0; _target = 0; }               // считать текущее положение нулём
  long stepsPerRevolution() const { return (long)(_stepsPerRev + 0.5f); }

  // ---- Настройки -----------------------------------------------------------------
  void setSpeedRPM(float rpm);                            // скорость, об/мин (по умолчанию 10, максимум ~15)
  void setAcceleration(float rpmPerSecond);               // плавность разгона, об/мин за секунду (по умолчанию 15); 0 = без разгона
  void reverse(bool on = true) { _reversed = on; }        // поменять направление вместо перепайки
  void setAutoRelease(uint16_t ms) { _releaseMs = ms; }   // через сколько мс покоя отключить катушки (0 = держать всегда). По умолчанию 500
  void release();                                         // отключить катушки (мотор не греется и не держит вал)
  void setGearRatio(float ratio);                         // передаточное число редуктора: у 28BYJ-48 63.68395 (по умолчанию) или 64; у остальных мотор-редукторов (по умолчанию 1)

  // ---- Драйвер STEP/DIR и биполярные моторы ----------------------------------------------------------
  void setEnablePin(uint8_t pin, bool activeLow = true);   // вывод EN драйвера: включается на время движения (A4988, DRV8825 включаются уровнем LOW)
  void setPulseWidthUs(uint16_t us);                       // длина импульса STEP, мкс (A4988 от 1, DRV8825 от 1.9, TB6600 от 2.5-5); по умолчанию 4
  bool setMicrosteps(uint16_t n);                          // 1, 2, 4, 8, 16, 32: шагов на оборот станет полных * n; если заданы пины MS, выставит их
  void setMicrostepPins(uint8_t ms1, uint8_t ms2, uint8_t ms3, MicrostepChip chip);   // пины MS1..MS3 (у DRV8825 M0..M2); если микрошаг выставлен перемычками, не нужно
  void setSpeedSteps(float stepsPerSecond);               // скорость сразу в шагах в секунду (для быстрых моторов)
  void setAccelerationSteps(float stepsPerSecond2);       // разгон в шагах/с²
  Driver driver() const { return _driver; }
  uint16_t microsteps() const { return _micro; }

  // ---- Диагностика -----------------------------------------------------------------
  void setLanguage(Language lang) { _lang = lang; }
  Error error() const { return _err; }
  void clearError() { _err = ERR_OK; }
  const __FlashStringHelper *errorText();
  void test();                                            // коротко повернуть вперёд и назад: проверить подключение (блокирует)
  void diagnose(Print &out);

 private:
  uint8_t _p[4];
  Driver _driver = DRIVER_ULN2003;
  uint16_t _fullSteps = 200, _micro = 1, _pulseUs = 4;
  uint8_t _en = 255, _ms[3] = {255, 255, 255}; bool _enLow = true; MicrostepChip _msChip = MS_A4988;
  bool _dirLevel = false, _spsMode = false;
  float _spsSet = 0, _accelSet2 = -1;
  Error _err = ERR_OK;
  Mode _mode = HALF_STEP;
  Language _lang = LANG_RU;
  float _gear = 63.68395f;
  float _stepsPerRev = 4075.7728f;
  float _rpm = 10, _accelRpmS = 15;
  float _maxSps = 679, _accelSps2 = 0, _v0 = 100;
  bool _reversed = false;
  uint16_t _releaseMs = 500;

  long _pos = 0, _target = 0, _startPos = 0;
  int8_t _dir = 1;
  bool _moving = false, _energized = false, _continuous = false;
  uint8_t _phase = 0;
  uint32_t _lastStepAt = 0, _intervalUs = 0, _idleSince = 0;

  void updateSpeeds();
  void applyPhase();
  void startMove(long newTarget);
  uint32_t intervalFor(long done, long remaining);
};

#endif  // ALASH_STEPPER_H
