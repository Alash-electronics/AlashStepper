# AlashStepper

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для шагового двигателя **28BYJ-48** с платой **ULN2003** (он есть почти в каждом наборе): повернуть на угол, на оборот, крутить без остановки, плавно разгоняться и тормозить. Без зависимостей. Сделана для уроков и наборов Alash Electronics.

```cpp
#include <AlashStepper.h>
AlashStepper motor(8, 9, 10, 11);      // IN1, IN2, IN3, IN4

void setup() { motor.begin(); }
void loop() {
  motor.moveDegrees(90);               // повернуть на 90 градусов
  motor.wait();                        // дождаться конца
  delay(1000);
}
```

### Что умеет
- **Пины по порядку IN1, IN2, IN3, IN4**: никаких «8, 10, 9, 11», как в старых примерах Stepper.
- **Углы и обороты:** `moveDegrees(90)`, `moveTo(180)`, `goToAngle(350)` (кратчайшим путём), `rotate(0.5)`, `moveSteps(n)`.
- **Настоящие 360 градусов:** у редуктора мотора передаточное число 63.68395, а не 64, поэтому оборот это 4076 шагов в полушаге. Если у вашего мотора 64:1, вызовите `setGearRatio(64)`.
- **Без блокировки:** `run()` в `loop()` делает шаги по таймеру, пока вы мигаете светодиодом и читаете датчики. Для простых скетчей есть `wait()`.
- **Плавный разгон и торможение** без пропуска шагов: `setSpeedRPM(10)` (максимум около 15), `setAcceleration()`.
- **Непрерывное вращение:** `spin()`, плавная остановка `stop()`, сразу `stopNow()`.
- **Мотор не греется:** через 0,5 с покоя катушки отключаются (`setAutoRelease()`, `release()`).
- **Положение:** `position()`, `positionDegrees()`, `setZero()`. Направление меняется `reverse()`, без перепайки.
- **Понятные подсказки** на русском, казахском и английском: `diagnose(Serial)` повернёт мотор и скажет, что проверить.
- Работает на **Arduino AVR, ESP32, ESP8266**.

### Подключение
| Плата ULN2003 | Uno / Nano | ESP32 |
|---|---|---|
| IN1 | D8 | GPIO16 |
| IN2 | D9 | GPIO17 |
| IN3 | D10 | GPIO18 |
| IN4 | D11 | GPIO19 |
| + (питание) | 5 В (лучше отдельный источник) | 5 В |
| - | GND | GND |

### Чем отличается от других библиотек
Мы посмотрели Stepper (встроенная), AccelStepper (Mike McCauley), CheapStepper (Tyler Henry) и Stepper_28BYJ_48:
- Встроенная Stepper не умеет полушаг и требует порядок пинов 8, 10, 9, 11: это главная причина, почему у новичков мотор «дёргается». У нас пины идут по порядку.
- Встроенная Stepper блокирует программу, а AccelStepper требует разобраться с режимами, шагами на оборот и ускорением. У нас `moveDegrees(90)`.
- Стандартные примеры считают оборот как 4096 шагов, и стрелка «уезжает» на полградуса за оборот; у нас используется настоящий ход редуктора.
- Ни в одной нет подсказок при неправильном подключении: у нас `diagnose()`.

### Примеры
`Basic`, `Angles`, `Continuous`, `Clock`, `Diagnose`.

### Тесты
Последовательность катушек, шаги на оборот, скорость, разгон и торможение, кратчайший путь, автоотключение проверены без железа: `extras/test`.

---

## Қазақша

**ULN2003** тақтасы бар **28BYJ-48** қадамдық қозғалтқышқа арналған қарапайым кітапхана (бұл әрбір жинақта бар): бұрышқа, айналымға бұру, тоқтамай айналдыру, жайлап үдеу және тежеу. Тәуелділіксіз. Alash Electronics сабақтары мен жинақтарына арналған.

```cpp
#include <AlashStepper.h>
AlashStepper motor(8, 9, 10, 11);      // IN1, IN2, IN3, IN4

void setup() { motor.begin(); }
void loop() {
  motor.moveDegrees(90);               // 90 градусқа бұру
  motor.wait();                        // аяқталғанша күту
  delay(1000);
}
```

### Мүмкіндіктері
- **Пиндер IN1, IN2, IN3, IN4 реті бойынша**: ескі Stepper мысалдарындағы «8, 10, 9, 11» жоқ.
- **Бұрыштар мен айналымдар:** `moveDegrees(90)`, `moveTo(180)`, `goToAngle(350)`, `rotate(0.5)`, `moveSteps(n)`.
- **Нақты 360 градус:** редуктордың беріліс саны 63.68395. Мотор 64:1 болса, `setGearRatio(64)`.
- **Бөгемей:** `loop()` ішіндегі `run()` қадамдарды таймер бойынша жасайды. Қарапайым скетчтерге `wait()` бар.
- **Жайлап үдеу және тежеу:** `setSpeedRPM(10)` (ең көбі шамамен 15), `setAcceleration()`.
- **Үздіксіз айналу:** `spin()`, `stop()`, `stopNow()`.
- **Мотор қызбайды:** 0,5 с тыныштықтан кейін катушкалар өшеді.
- **Түсінікті кеңестер** орысша, қазақша, ағылшынша: `diagnose(Serial)`.
- **Arduino AVR, ESP32, ESP8266** тақталарында жұмыс істейді.

### Қосу
| ULN2003 тақтасы | Uno / Nano | ESP32 |
|---|---|---|
| IN1 | D8 | GPIO16 |
| IN2 | D9 | GPIO17 |
| IN3 | D10 | GPIO18 |
| IN4 | D11 | GPIO19 |
| + (қуат) | 5 В (бөлек көз жақсы) | 5 В |
| - | GND | GND |

### Мысалдар
`Basic`, `Angles`, `Continuous`, `Clock`, `Diagnose`.

---

## English

A simple library for the **28BYJ-48** stepper motor with a **ULN2003** board (found in almost every kit): turn by an angle or a number of turns, spin without stopping, accelerate and brake smoothly. No dependencies. Made for classes and Alash Electronics kits.

```cpp
#include <AlashStepper.h>
AlashStepper motor(8, 9, 10, 11);      // IN1, IN2, IN3, IN4

void setup() { motor.begin(); }
void loop() {
  motor.moveDegrees(90);               // turn 90 degrees
  motor.wait();                        // wait until done
  delay(1000);
}
```

### Features
- **Pins in plain IN1, IN2, IN3, IN4 order**: no "8, 10, 9, 11" trick from old Stepper examples.
- **Angles and turns:** `moveDegrees(90)`, `moveTo(180)`, `goToAngle(350)` (shortest way), `rotate(0.5)`, `moveSteps(n)`.
- **A true 360 degrees:** the motor gearbox is 63.68395:1, not 64, so one turn is 4076 half-steps. If your motor is 64:1, call `setGearRatio(64)`.
- **Non-blocking:** `run()` in `loop()` steps on a timer while you blink LEDs and read sensors. `wait()` is there for simple sketches.
- **Smooth acceleration and braking** without lost steps: `setSpeedRPM(10)` (max about 15), `setAcceleration()`.
- **Continuous rotation:** `spin()`, smooth `stop()`, instant `stopNow()`.
- **The motor stays cool:** the coils switch off after 0.5 s of rest.
- **Clear tips** in Russian, Kazakh and English: `diagnose(Serial)` turns the motor and says what to check.
- Works on **Arduino AVR, ESP32, ESP8266**.

### Wiring
| ULN2003 board | Uno / Nano | ESP32 |
|---|---|---|
| IN1 | D8 | GPIO16 |
| IN2 | D9 | GPIO17 |
| IN3 | D10 | GPIO18 |
| IN4 | D11 | GPIO19 |
| + (power) | 5 V (a separate supply is better) | 5 V |
| - | GND | GND |

### Examples
`Basic`, `Angles`, `Continuous`, `Clock`, `Diagnose`.

### License
MIT, Alash Electronics.
