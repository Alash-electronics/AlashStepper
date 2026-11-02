// [ru] AlashStepper: биполярный мотор (4 провода) через два H-моста: L298N, L293D, TB6612, DRV8833. Управляются четыре входа IN1..IN4 драйвера.
//      Подключение L298N: IN1, IN2 на пины 8, 9 (первая обмотка), IN3, IN4 на пины 10, 11 (вторая обмотка); перемычки ENA и ENB оставьте на месте;
//      питание мотора 7-12 В на +12V, GND общий с Arduino. Две обмотки мотора определяются прозвонкой: у каждой пары проводов сопротивление несколько Ом.
//      Если мотор дёргается вместо вращения, поменяйте местами провода одной обмотки.
// [kk] AlashStepper: екі H-көпір арқылы биполярлы мотор (4 сым): L298N, L293D, TB6612, DRV8833. Драйверді IN1..IN4 төрт кірісі басқарады.
//      L298N қосу: IN1, IN2 8, 9 пиндерге (бірінші орам), IN3, IN4 10, 11 пиндерге (екінші орам); ENA және ENB перемычкаларын орнында қалдырыңыз;
//      мотор қуаты 7-12 В +12V-ге, GND Arduino-мен ортақ. Мотордың екі орамы қоңыраумен анықталады: әр сым жұбының кедергісі бірнеше Ом.
//      Мотор айналудың орнына тартылса, бір орам сымдарын ауыстырыңыз.
// [en] AlashStepper: a bipolar motor (4 wires) through two H-bridges: L298N, L293D, TB6612, DRV8833. The four inputs IN1..IN4 of the driver are controlled.
//      L298N wiring: IN1, IN2 to pins 8, 9 (the first winding), IN3, IN4 to pins 10, 11 (the second winding); leave the ENA and ENB jumpers in place;
//      motor power 7-12 V on +12V, GND common with the Arduino. Find the two windings by continuity: each wire pair has a few ohms.
//      If the motor jitters instead of turning, swap the wires of one winding.
#include <AlashStepper.h>

AlashStepper motor(AlashStepper::DRIVER_BIPOLAR_4WIRE, 8, 9, 10, 11, 200);   // тип драйвера, A1, A2, B1, B2, полных шагов на оборот

void setup() {
  Serial.begin(115200);
  motor.begin(AlashStepper::HALF_STEP);          // полушаг: 400 шагов на оборот и плавнее; FULL_STEP: 200 и сильнее
  motor.setSpeedRPM(30);
  motor.setAcceleration(60);
}

void loop() {
  motor.rotate(1);
  motor.wait();
  delay(500);
  motor.moveDegrees(-180);
  motor.wait();
  delay(500);
}
