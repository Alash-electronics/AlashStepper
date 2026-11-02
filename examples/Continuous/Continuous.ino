// [ru] AlashStepper: непрерывное вращение. Мотор крутится 5 секунд в одну сторону, плавно останавливается,
//      потом 5 секунд в другую.
// [kk] AlashStepper: үздіксіз айналу. Мотор 5 секунд бір жаққа айналады, баяу тоқтайды,
//      сосын 5 секунд екінші жаққа айналады.
// [en] AlashStepper: continuous rotation. The motor spins one way for 5 seconds, stops smoothly,
//      then spins the other way for 5 seconds.
#include <AlashStepper.h>

// Пины IN1..IN4 платы ULN2003: Uno / Nano  8, 9, 10, 11;  ESP32  16, 17, 18, 19;  ESP8266  D1, D2, D5, D6.
#if defined(ESP32)
AlashStepper motor(16, 17, 18, 19);
#elif defined(ESP8266)
AlashStepper motor(5, 4, 14, 12);
#else
AlashStepper motor(8, 9, 10, 11);
#endif
uint32_t changedAt = 0;
bool forward = true;
bool stopping = false;

void setup() {
  motor.begin();
  motor.setSpeedRPM(10);
  motor.spin(forward);
  changedAt = millis();
}

void loop() {
  motor.run();
  if (!stopping && millis() - changedAt > 5000) {
    motor.stop();                         // плавное торможение
    stopping = true;
  }
  if (stopping && !motor.isMoving()) {    // остановился: разворачиваемся
    forward = !forward;
    motor.spin(forward);
    changedAt = millis();
    stopping = false;
  }
}
