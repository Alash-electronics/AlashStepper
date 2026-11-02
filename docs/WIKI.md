# AlashStepper: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для шагового двигателя **28BYJ-48** с платой **ULN2003** (он есть почти в каждом наборе): повернуть на угол, на оборот, крутить без остановки, плавно разгоняться и тормозить. Без зависимостей. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashStepper** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Подключение
| Плата ULN2003 | Uno / Nano | ESP32 |
|---|---|---|
| IN1 | D8 | GPIO16 |
| IN2 | D9 | GPIO17 |
| IN3 | D10 | GPIO18 |
| IN4 | D11 | GPIO19 |
| + (питание) | 5 В (лучше отдельный источник) | 5 В |
| - | GND | GND |

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `begin(mode)` | Запуск: HALF_STEP (плавно, по умолчанию) или FULL_STEP (сильнее) |
| `moveDegrees(a) / moveTo(a)` | Повернуть на угол / в положение |
| `goToAngle(a)` | В угол 0..360 кратчайшим путём |
| `rotate(turns) / moveSteps(n)` | Обороты / шаги |
| `spin(forward) / stop() / stopNow()` | Вращение без остановки, плавная и резкая остановка |
| `run() / wait() / isMoving()` | Делать шаги в loop() / дождаться конца / движется ли |
| `position() / positionDegrees() / setZero()` | Положение и ноль |
| `setSpeedRPM(rpm) / setAcceleration(a)` | Скорость и плавность разгона |
| `reverse()` | Поменять направление |
| `release() / setAutoRelease(ms)` | Отключить катушки (мотор не греется) |
| `test() / diagnose(Serial)` | Проверка подключения и самопроверка |

### Если что-то не работает
**Мотор дёргается, но не вращается**

Порядок проводов IN1..IN4 должен совпадать с порядком пинов в конструкторе. Запустите `stepper.test()`. Питание лучше отдельное, 5 В от платы ULN2003, не от пина Arduino.

**Мотор вращается в обратную сторону**

Вызовите `stepper.reverse()`.

**Мотор пропускает шаги на большой скорости**

28BYJ-48 держит до 15 об/мин. Уменьшите `setSpeedRPM()` и включите разгон `setAcceleration()`.

### Чем отличается от других библиотек
Мы посмотрели Stepper (встроенная), AccelStepper (Mike McCauley), CheapStepper (Tyler Henry) и Stepper_28BYJ_48:
- Встроенная Stepper не умеет полушаг и требует порядок пинов 8, 10, 9, 11: это главная причина, почему у новичков мотор «дёргается». У нас пины идут по порядку.
- Встроенная Stepper блокирует программу, а AccelStepper требует разобраться с режимами, шагами на оборот и ускорением. У нас `moveDegrees(90)`.
- Стандартные примеры считают оборот как 4096 шагов, и стрелка «уезжает» почти на 2 градуса за оборот; у нас используется настоящее передаточное число редуктора.
- Ни в одной нет подсказок при неправильном подключении: у нас `diagnose()`.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `Angles`, `Continuous`, `Clock`, `Diagnose`.

---

## Қазақша

**ULN2003** тақтасы бар **28BYJ-48** қадамдық қозғалтқышқа арналған қарапайым кітапхана (бұл әрбір жинақта бар): бұрышқа, айналымға бұру, тоқтамай айналдыру, жайлап үдеу және тежеу. Тәуелділіксіз. Alash Electronics сабақтары мен жинақтарына арналған.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashStepper** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Қосу
| ULN2003 тақтасы | Uno / Nano | ESP32 |
|---|---|---|
| IN1 | D8 | GPIO16 |
| IN2 | D9 | GPIO17 |
| IN3 | D10 | GPIO18 |
| IN4 | D11 | GPIO19 |
| + (қуат) | 5 В (бөлек көз жақсы) | 5 В |
| - | GND | GND |

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `begin(mode)` | Іске қосу: HALF_STEP не FULL_STEP |
| `moveDegrees(a) / moveTo(a)` | Бұрышқа / орынға бұру |
| `goToAngle(a)` | 0..360 бұрышқа қысқа жолмен |
| `rotate(turns) / moveSteps(n)` | Айналым / қадамдар |
| `spin(forward) / stop() / stopNow()` | Тоқтаусыз айналу, тегіс және тез тоқтау |
| `run() / wait() / isMoving()` | loop() ішінде қадам / аяқталуын күту / қозғала ма |
| `position() / positionDegrees() / setZero()` | Орын және нөл |
| `setSpeedRPM(rpm) / setAcceleration(a)` | Жылдамдық және тегіс үдеу |
| `reverse()` | Бағытты ауыстыру |
| `release() / setAutoRelease(ms)` | Катушкаларды өшіру (қозғалтқыш қызбайды) |
| `test() / diagnose(Serial)` | Қосылымды тексеру және өзін тексеру |

### Бірдеңе жұмыс істемесе
**Қозғалтқыш тітіркенеді, бірақ айналмайды**

IN1..IN4 сымдарының реті конструктордағы пиндер ретімен бірдей болуы керек. `stepper.test()` іске қосыңыз. Қорек бөлек болғаны жөн.

**Қозғалтқыш кері айналады**

`stepper.reverse()` шақырыңыз.

**Қозғалтқыш жоғары жылдамдықта қадам өткізеді**

28BYJ-48 15 айн/мин-қа дейін ұстайды. `setSpeedRPM()` азайтып, `setAcceleration()` қосыңыз.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `Angles`, `Continuous`, `Clock`, `Diagnose`.

---

## English

A simple library for the **28BYJ-48** stepper motor with a **ULN2003** board (found in almost every kit): turn by an angle or a number of turns, spin without stopping, accelerate and brake smoothly. No dependencies. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashStepper** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Wiring
| ULN2003 board | Uno / Nano | ESP32 |
|---|---|---|
| IN1 | D8 | GPIO16 |
| IN2 | D9 | GPIO17 |
| IN3 | D10 | GPIO18 |
| IN4 | D11 | GPIO19 |
| + (power) | 5 V (a separate supply is better) | 5 V |
| - | GND | GND |

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `begin(mode)` | Start: HALF_STEP (smooth, default) or FULL_STEP (stronger) |
| `moveDegrees(a) / moveTo(a)` | Turn by an angle / to a position |
| `goToAngle(a)` | To an angle 0..360 by the shortest way |
| `rotate(turns) / moveSteps(n)` | Turns / steps |
| `spin(forward) / stop() / stopNow()` | Continuous spin, soft and hard stop |
| `run() / wait() / isMoving()` | Step in loop() / wait for the end / is it moving |
| `position() / positionDegrees() / setZero()` | Position and zero |
| `setSpeedRPM(rpm) / setAcceleration(a)` | Speed and smooth start |
| `reverse()` | Swap direction |
| `release() / setAutoRelease(ms)` | Turn the coils off (the motor stays cool) |
| `test() / diagnose(Serial)` | Wiring test and self-check |

### If something does not work
**The motor twitches but does not turn**

The IN1..IN4 wires must match the pin order in the constructor. Run `stepper.test()`. A separate 5 V supply for the ULN2003 board is better than an Arduino pin.

**The motor turns the wrong way**

Call `stepper.reverse()`.

**The motor skips steps at high speed**

The 28BYJ-48 holds up to 15 rpm. Lower `setSpeedRPM()` and use `setAcceleration()`.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `Angles`, `Continuous`, `Clock`, `Diagnose`.

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashStepper
