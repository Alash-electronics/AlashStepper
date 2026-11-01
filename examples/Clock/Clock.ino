// AlashStepper: секундная стрелка. Мотор делает один оборот за минуту: каждую секунду поворот на 6 градусов.
// Точность по таймеру Arduino (кварц на Uno точный примерно до минуты в сутки). Для настоящих часов нужен модуль DS3231.
#include <AlashStepper.h>

// Пины IN1..IN4 платы ULN2003: Uno / Nano  8, 9, 10, 11;  ESP32  16, 17, 18, 19;  ESP8266  D1, D2, D5, D6.
#if defined(ESP32)
AlashStepper motor(16, 17, 18, 19);
#elif defined(ESP8266)
AlashStepper motor(5, 4, 14, 12);
#else
AlashStepper motor(8, 9, 10, 11);
#endif
uint32_t lastTick = 0;

void setup() {
  motor.begin();
  motor.setSpeedRPM(5);
}

void loop() {
  motor.run();
  if (millis() - lastTick >= 1000) {
    lastTick += 1000;                     // += вместо = millis(), чтобы секунды не накапливали ошибку
    motor.moveDegrees(6);                 // 360 градусов / 60 секунд
  }
}
