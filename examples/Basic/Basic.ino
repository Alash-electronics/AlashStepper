// AlashStepper: самый простой пример. Мотор поворачивается на 90 градусов и обратно.
//
// Подключение: плата ULN2003 IN1..IN4 на четыре пина (см. ниже), мотор 28BYJ-48 в белый разъём,
// питание платы 5 В (лучше отдельный источник), GND общий с Arduino.
#include <AlashStepper.h>

// Пины IN1..IN4 платы ULN2003: Uno / Nano  8, 9, 10, 11;  ESP32  16, 17, 18, 19;  ESP8266  D1, D2, D5, D6.
#if defined(ESP32)
AlashStepper motor(16, 17, 18, 19);
#elif defined(ESP8266)
AlashStepper motor(5, 4, 14, 12);
#else
AlashStepper motor(8, 9, 10, 11);
#endif

void setup() {
  motor.begin();
}

void loop() {
  motor.moveDegrees(90);                  // повернуть на 90 градусов
  motor.wait();                           // дождаться конца поворота
  delay(1000);
  motor.moveDegrees(-90);                 // обратно
  motor.wait();
  delay(1000);
}
