// [ru] AlashStepper: самый простой пример. Мотор поворачивается на 90 градусов и обратно.
//
//      Подключение: плата ULN2003 IN1..IN4 на четыре пина (см. ниже), мотор 28BYJ-48 в белый разъём,
//      питание платы 5 В (лучше отдельный источник), GND общий с Arduino.
// [kk] AlashStepper: ең қарапайым мысал. Мотор 90 градусқа бұрылады да, кері қайтады.
//
//      Қосу: ULN2003 тақшасының IN1..IN4 төрт пинге (төменде қараңыз), 28BYJ-48 моторы ақ қосқышқа,
//      тақша қуаты 5 В (бөлек көз дұрыс), GND Arduino-мен ортақ.
// [en] AlashStepper: the simplest example. The motor turns 90 degrees and back.
//
//      Wiring: the ULN2003 board IN1..IN4 to four pins (see below), the 28BYJ-48 motor into the white connector,
//      board power 5 V (better a separate supply), GND common with the Arduino.
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
