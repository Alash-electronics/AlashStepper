// [ru] AlashStepper: мотор не крутится или дёргается? Скетч повернёт мотор на 45 градусов вперёд и назад
//      и подскажет, что проверить.
// [kk] AlashStepper: мотор айналмай ма немесе тартылып тұр ма? Скетч моторды 45 градусқа алға және артқа бұрады
//      және нені тексеру керегін айтады.
// [en] AlashStepper: the motor does not turn or jitters? The sketch turns the motor 45 degrees forward and back
//      and tells what to check.
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
  Serial.begin(115200);
  delay(500);
  motor.setLanguage(AlashStepper::LANG_RU);   // LANG_KK: қазақша, LANG_EN: English
  motor.begin();
  motor.diagnose(Serial);
}

void loop() {}
