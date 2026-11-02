# AlashStepper: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **шаговых двигателей с любым драйвером**: **28BYJ-48 + ULN2003** (он есть почти в каждом наборе), **NEMA17 через A4988 / DRV8825 / TMC2208 / TB6600 (STEP/DIR)** и биполярные моторы через **L298N / L293D / TB6612**. Повернуть на угол, на оборот, крутить без остановки, плавно разгоняться и тормозить. Без зависимостей. Сделана для уроков и наборов Alash Electronics.

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
- **Три типа драйверов:** ULN2003 (по умолчанию), STEP/DIR (микрошаги, вывод EN, длина импульса) и два H-моста с четырьмя проводами.
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
| `AlashStepper(DRIVER_STEP_DIR, step, dir, steps)` | Драйвер STEP/DIR (A4988, DRV8825, TMC2208, TB6600), полных шагов на оборот |
| `AlashStepper(DRIVER_BIPOLAR_4WIRE, a1, a2, b1, b2, steps)` | Биполярный мотор через два H-моста (L298N, L293D, TB6612) |
| `setEnablePin(pin, activeLow) / setPulseWidthUs(us)` | Вывод EN драйвера и длина импульса STEP |
| `setMicrosteps(n) / setMicrostepPins(ms1, ms2, ms3, chip)` | Микрошаг 1..32 и выбор его пинами (A4988, DRV8825) |
| `setSpeedSteps(sps) / setAccelerationSteps(a)` | Скорость и разгон в шагах (для быстрых моторов) |
| `error() / errorText()` | Код ошибки и сообщение на ru/kk/en |
| `test() / diagnose(Serial)` | Проверка подключения и самопроверка |

### Если что-то не работает
**Мотор дёргается, но не вращается**

Порядок проводов IN1..IN4 должен совпадать с порядком пинов в конструкторе. Запустите `stepper.test()`. Питание лучше отдельное, 5 В от платы ULN2003, не от пина Arduino.

**Мотор вращается в обратную сторону**

Вызовите `stepper.reverse()`.

**Мотор пропускает шаги на большой скорости**

28BYJ-48 держит до 15 об/мин. Уменьшите `setSpeedRPM()` и включите разгон `setAcceleration()`.

**NEMA17 с A4988 / DRV8825 не вращается или греется**

Проверьте питание драйвера: 8-35 В на VMOT и GND и конденсатор не менее 47 мкФ между ними. Вывод EN включается уровнем LOW (`setEnablePin()` или соедините EN с GND), RESET и SLEEP должны быть соединены. Настройте ток подстроечным резистором (Vref) и проверьте порядок обмоток мотора. Никогда не отключайте мотор от включённого драйвера.

**Мотор на L298N дёргается вместо вращения**

Обмотки подключены в неправильном порядке: пины a1, a2 должны идти на одну обмотку, b1, b2 на другую. Поменяйте местами провода одной обмотки или порядок пинов. Перемычки ENA и ENB на L298N не снимайте (или подайте на них HIGH).

**Шагов на оборот не сходятся с реальными**

Для STEP/DIR число шагов = полных шагов мотора * микрошаг * передаточное число. Проверьте перемычки MS1..MS3 на драйвере и вызовите `setMicrosteps()` с тем же значением, и `setGearRatio()`, если есть редуктор.

### Чем отличается от других библиотек
Мы посмотрели Stepper (встроенная), AccelStepper (Mike McCauley), CheapStepper (Tyler Henry) и Stepper_28BYJ_48:
- Встроенная Stepper не умеет полушаг и требует порядок пинов 8, 10, 9, 11: это главная причина, почему у новичков мотор «дёргается». У нас пины идут по порядку.
- Встроенная Stepper блокирует программу, а AccelStepper требует разобраться с режимами, шагами на оборот и ускорением. У нас `moveDegrees(90)`.
- Стандартные примеры считают оборот как 4096 шагов, и стрелка «уезжает» почти на 2 градуса за оборот; у нас используется настоящее передаточное число редуктора.
- AccelStepper умеет разные драйверы, но без углов, микрошагов по пинам и подсказок; у нас один интерфейс для ULN2003, STEP/DIR и H-мостов.
- Ни в одной нет подсказок при неправильном подключении: у нас `diagnose()`.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `Angles`, `Continuous`, `Clock`, `StepDir_A4988`, `Bipolar_L298N`, `Diagnose`.

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
- **Драйверлердің үш түрі:** ULN2003 (әдепкі), STEP/DIR (A4988, DRV8825, TMC2208, TB6600: микроқадам, EN шықпасы, импульс ұзындығы) және төрт сымды екі H-көпір (L298N, L293D, TB6612). Мысалы: `AlashStepper nema(AlashStepper::DRIVER_STEP_DIR, 2, 3, 200);`
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
| `AlashStepper(DRIVER_STEP_DIR, step, dir, steps)` | STEP/DIR драйвері |
| `AlashStepper(DRIVER_BIPOLAR_4WIRE, a1, a2, b1, b2, steps)` | Екі H-көпір арқылы биполярлы мотор |
| `setEnablePin(pin, activeLow) / setPulseWidthUs(us)` | EN шықпасы және STEP импульсі |
| `setMicrosteps(n) / setMicrostepPins(ms1, ms2, ms3, chip)` | Микроқадам 1..32 |
| `setSpeedSteps(sps) / setAccelerationSteps(a)` | Қадамдағы жылдамдық және үдеу |
| `error() / errorText()` | Қате коды және хабарлама |
| `test() / diagnose(Serial)` | Қосылымды тексеру және өзін тексеру |

### Бірдеңе жұмыс істемесе
**Қозғалтқыш тітіркенеді, бірақ айналмайды**

IN1..IN4 сымдарының реті конструктордағы пиндер ретімен бірдей болуы керек. `stepper.test()` іске қосыңыз. Қорек бөлек болғаны жөн.

**Қозғалтқыш кері айналады**

`stepper.reverse()` шақырыңыз.

**Қозғалтқыш жоғары жылдамдықта қадам өткізеді**

28BYJ-48 15 айн/мин-қа дейін ұстайды. `setSpeedRPM()` азайтып, `setAcceleration()` қосыңыз.

**A4988 / DRV8825 бар NEMA17 айналмайды немесе қызады**

Драйвер қуатын тексеріңіз: VMOT және GND-ға 8-35 В және олардың арасында кемінде 47 мкФ конденсатор. EN шықпасы LOW деңгейімен қосылады (`setEnablePin()` немесе EN-ді GND-ға жалғаңыз), RESET және SLEEP жалғануы керек. Токты реттегіш резистормен (Vref) баптаңыз.

**L298N-дегі мотор айналудың орнына тартылады**

Орамдар дұрыс емес ретпен қосылған: a1, a2 пиндері бір орамға, b1, b2 екіншісіне баруы керек. Бір орам сымдарын немесе пиндер ретін ауыстырыңыз.

**Бір айналымға қадамдар шынайымен сәйкес келмейді**

STEP/DIR үшін қадам саны = мотордың толық қадамдары * микроқадам * беріліс саны. Драйверде MS1..MS3 перемычкаларын тексеріп, сол мәнмен `setMicrosteps()` шақырыңыз.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `Angles`, `Continuous`, `Clock`, `StepDir_A4988`, `Bipolar_L298N`, `Diagnose`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

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
- **Three driver types:** ULN2003 (default), STEP/DIR (A4988, DRV8825, TMC2208, TB6600: microsteps, the EN pin, pulse width) and two H-bridges with four wires (L298N, L293D, TB6612). For example: `AlashStepper nema(AlashStepper::DRIVER_STEP_DIR, 2, 3, 200);`
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
| `AlashStepper(DRIVER_STEP_DIR, step, dir, steps)` | A STEP/DIR driver (A4988, DRV8825, TMC2208, TB6600), full steps per revolution |
| `AlashStepper(DRIVER_BIPOLAR_4WIRE, a1, a2, b1, b2, steps)` | A bipolar motor through two H-bridges (L298N, L293D, TB6612) |
| `setEnablePin(pin, activeLow) / setPulseWidthUs(us)` | The driver EN pin and the STEP pulse width |
| `setMicrosteps(n) / setMicrostepPins(ms1, ms2, ms3, chip)` | Microstep 1..32 and selecting it by pins (A4988, DRV8825) |
| `setSpeedSteps(sps) / setAccelerationSteps(a)` | Speed and acceleration in steps (for fast motors) |
| `error() / errorText()` | The error code and message in ru/kk/en |
| `test() / diagnose(Serial)` | Wiring test and self-check |

### If something does not work
**The motor twitches but does not turn**

The IN1..IN4 wires must match the pin order in the constructor. Run `stepper.test()`. A separate 5 V supply for the ULN2003 board is better than an Arduino pin.

**The motor turns the wrong way**

Call `stepper.reverse()`.

**The motor skips steps at high speed**

The 28BYJ-48 holds up to 15 rpm. Lower `setSpeedRPM()` and use `setAcceleration()`.

**A NEMA17 with an A4988 / DRV8825 does not turn or gets hot**

Check the driver power: 8-35 V on VMOT and GND with a capacitor of at least 47 uF between them. EN is active LOW (`setEnablePin()` or join EN to GND), RESET and SLEEP must be joined. Set the current with the Vref trimmer and check the motor winding order. Never disconnect the motor from a powered driver.

**A motor on an L298N jitters instead of turning**

The windings are connected in the wrong order: pins a1, a2 must go to one winding and b1, b2 to the other. Swap the wires of one winding or the pin order. Do not remove the ENA and ENB jumpers on the L298N (or give them HIGH).

**Steps per revolution do not match the real ones**

For STEP/DIR the step count = the motor full steps * the microstep * the gear ratio. Check the MS1..MS3 jumpers on the driver and call `setMicrosteps()` with the same value, and `setGearRatio()` if there is a gearbox.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `Angles`, `Continuous`, `Clock`, `StepDir_A4988`, `Bipolar_L298N`, `Diagnose`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashStepper
