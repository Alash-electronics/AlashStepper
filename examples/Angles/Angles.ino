// [ru] AlashStepper: углы без остановки программы. Мотор поворачивается, а светодиод всё это время мигает.
//      run() нужно вызывать в loop() как можно чаще: он делает шаг, когда пора.
// [kk] AlashStepper: бағдарламаны тоқтатпай бұрыштар. Мотор бұрылады, осы уақытта жарықдиод жыпылықтап тұрады.
//      run() функциясын loop() ішінде мүмкіндігінше жиі шақыру керек: ол уақыты келгенде қадам жасайды.
// [en] AlashStepper: angles without stopping the program. The motor turns while an LED blinks all that time.
//      run() must be called in loop() as often as possible: it makes a step when it is time.
#include <AlashStepper.h>

// Пины IN1..IN4 платы ULN2003: Uno / Nano  8, 9, 10, 11;  ESP32  16, 17, 18, 19;  ESP8266  D1, D2, D5, D6.
#if defined(ESP32)
AlashStepper motor(16, 17, 18, 19);
#elif defined(ESP8266)
AlashStepper motor(5, 4, 14, 12);
#else
AlashStepper motor(8, 9, 10, 11);
#endif
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif
uint32_t lastBlink = 0, lastTurn = 0;
const float angles[] = {0, 90, 180, 270, 45};
uint8_t current = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  motor.begin();
  motor.setSpeedRPM(10);                  // об/мин (максимум около 15)
  motor.goToAngle(angles[0]);
}

void loop() {
  motor.run();                            // не блокирует, шаги делаются по таймеру

  if (!motor.isMoving() && millis() - lastTurn > 1000) {   // дошёл до угла, постоял секунду, едем к следующему
    lastTurn = millis();
    current = (current + 1) % (sizeof(angles) / sizeof(angles[0]));
    motor.goToAngle(angles[current]);       // кратчайшим путём
  }

  if (millis() - lastBlink > 200) {       // параллельно мигаем
    lastBlink = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
