// AlashStepper: шаговый двигатель 28BYJ-48 с драйвером ULN2003: углы, обороты, плавный разгон, без блокировки.
// Без зависимостей. MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashStepper
#ifndef ALASH_STEPPER_H
#define ALASH_STEPPER_H

#include <Arduino.h>

class AlashStepper {
 public:
  // HALF_STEP: 8 фаз, плавнее и точнее (по умолчанию, ~4076 шагов на оборот). FULL_STEP: 4 фазы, сильнее (~2038).
  enum Mode : uint8_t { HALF_STEP = 0, FULL_STEP = 1 };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  // in1..in4: пины Arduino, подключённые к входам IN1..IN4 платы ULN2003 в том же порядке (не перепутывайте, как в старых примерах Stepper).
  AlashStepper(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4);

  void begin(Mode mode = HALF_STEP);

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
  void setGearRatio(float ratio);                         // передаточное число редуктора: 63.68395 (по умолчанию) или 64

  // ---- Диагностика -----------------------------------------------------------------
  void setLanguage(Language lang) { _lang = lang; }
  void test();                                            // коротко повернуть вперёд и назад: проверить подключение (блокирует)
  void diagnose(Print &out);

 private:
  uint8_t _p[4];
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
