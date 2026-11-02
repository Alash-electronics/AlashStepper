// [ru] AlashStepper: двигатель NEMA17 (или любой биполярный) через драйвер STEP/DIR: A4988, DRV8825, TMC2208, TB6600 и подобные.
//      Подключение: STEP на пин 2, DIR на пин 3, EN на пин 4 (необязательно). VMOT и GND драйвера к блоку питания 8-35 В, между ними конденсатор не менее 47 мкФ,
//      VDD и GND к 5 В Arduino, RESET и SLEEP соединены между собой, обмотки мотора на 1A, 1B, 2A, 2B. Ток настройте подстроечным резистором (Vref).
//      Микрошаг задан перемычками MS1..MS3 на драйвере (здесь 1/16); если вы подключили MS1..MS3 к пинам Arduino, раскомментируйте setMicrostepPins().
// [kk] AlashStepper: NEMA17 қозғалтқышы (немесе кез келген биполярлы) STEP/DIR драйвері арқылы: A4988, DRV8825, TMC2208, TB6600 және соларға ұқсас.
//      Қосу: STEP 2 пинге, DIR 3 пинге, EN 4 пинге (міндетті емес). Драйвер VMOT және GND 8-35 В қуат блогына, арасында кемінде 47 мкФ конденсатор,
//      VDD және GND Arduino 5 В-қа, RESET және SLEEP өзара жалғанған, мотор орамдары 1A, 1B, 2A, 2B-ге. Токты реттегіш резистормен (Vref) баптаңыз.
//      Микроқадам драйверде MS1..MS3 перемычкаларымен берілген (мұнда 1/16); MS1..MS3 Arduino пиндеріне қосылса, setMicrostepPins() ашыңыз.
// [en] AlashStepper: a NEMA17 motor (or any bipolar one) through a STEP/DIR driver: A4988, DRV8825, TMC2208, TB6600 and the like.
//      Wiring: STEP to pin 2, DIR to pin 3, EN to pin 4 (optional). The driver VMOT and GND to a 8-35 V supply with a capacitor of at least 47 uF between them,
//      VDD and GND to the Arduino 5 V, RESET and SLEEP joined together, the motor windings to 1A, 1B, 2A, 2B. Set the current with the trimmer (Vref).
//      The microstep is set by the MS1..MS3 jumpers on the driver (1/16 here); if you wired MS1..MS3 to Arduino pins, uncomment setMicrostepPins().
#include <AlashStepper.h>

// тип драйвера, STEP, DIR, полных шагов на оборот (200 для 1.8 градуса, 400 для 0.9)
AlashStepper motor(AlashStepper::DRIVER_STEP_DIR, 2, 3, 200);

void setup() {
  Serial.begin(115200);
  motor.setEnablePin(4, true);                   // EN активен LOW; уберите строку, если EN не подключён
  // motor.setMicrostepPins(5, 6, 7, AlashStepper::MS_A4988);   // MS1..MS3 на пинах Arduino; для DRV8825 MS_DRV8825
  motor.begin();
  motor.setMicrosteps(16);                       // 1/16 шага: 3200 шагов на оборот
  motor.setSpeedRPM(60);                         // 60 об/мин
  motor.setAcceleration(120);                    // плавный разгон, об/мин за секунду
  if (motor.error() != AlashStepper::ERR_OK) Serial.println(motor.errorText());
}

void loop() {
  motor.rotate(1);                               // один оборот вперёд
  motor.wait();
  delay(500);
  motor.moveDegrees(-90);                        // 90 градусов назад
  motor.wait();
  delay(500);
}
