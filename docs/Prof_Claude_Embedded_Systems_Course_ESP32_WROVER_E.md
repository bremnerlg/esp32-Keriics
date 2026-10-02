# Professor Claude's Embedded Systems Course: ESP32-WROVER-E (Freenove kit) on ESP-IDF v6.1

## Course description

A self-paced, roughly 20-week computer engineering course taught in six stages (0-5), with a capstone project. It is built around three goals:

1. Learn the operating systems fundamentals that matter for real ESP32 firmware.
2. Take sensor input and turn it into something useful.
3. Communicate with the ESP32 over UART, Bluetooth and other practical protocols.

**Prerequisite:** you can already build, flash and monitor an ESP-IDF v6.1 project, blink an LED and drive or read a GPIO. The course starts after that.

**Hardware:** the board in your Freenove Ultimate Starter Kit for ESP32. The kit's own parts photo shows an **ESP32-WROVER-E** module, and Freenove's tutorial and sketches call the board "ESP32-WROVER". The datasheet is Espressif's [ESP32-WROVER-E & ESP32-WROVER-IE datasheet](https://documentation.espressif.com/esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf) (v2.4 when I read it).

Every ESP-IDF link points at the v6.1 docs, which describe ESP-IDF FreeRTOS as based on Vanilla FreeRTOS v10.5.1. Each OSTEP entry names the chapter title and filename, all under `https://pages.cs.wisc.edu/~remzi/OSTEP/`. Found-online items are readings, lectures and reference examples.

**How the assignments work now.** The assignments are built from the Freenove kit's own projects. The kit supplies the hardware, the circuit and the target behavior, but its code is Arduino (`setup()`/`loop()`, `Serial`, `ledcWrite`, third-party libraries). Your job is to rebuild each project in ESP-IDF, to a written spec, with measurements and a deliberately injected fault. Each assignment lists the Freenove source, an **Arduino → ESP-IDF swap list** of the functions that replace the Arduino calls, acceptance criteria, and a "break it" step. The heavier systems labs that carry through to the capstone are marked **(Prof. Claude)** and run on kit hardware too.

## Lab rules (how to stay out of tutorial hell)

1. **Spec first (the pre-lab).** Before writing code, write one paragraph stating what you will build and the measurable acceptance criteria (rates, error counts, recovery behavior).
2. **Design from the datasheet and the v6.1 reference, not from a tutorial.** Freenove's Arduino sketches and Espressif's examples are reference material, not assignments. Read the circuit and the behavior from Freenove, look up API usage in the v6.1 docs, then build without either open. Do not install Arduino-framework libraries (MPU6050_tockn, DHTesp, Keypad and so on) and do not wrap them: the point is to write the driver.
3. **Measure, don't eyeball (the demonstration).** "It seems to work" is not a result. Use the multimeter, your own counters, timestamps and, if you own one, the logic analyzer.
4. **Break it on purpose.** Every lab includes a fault you inject yourself: corrupted bytes, an unplugged sensor, a starved task, a hung task.
5. **Write a one-page report per lab:** the spec, the design, the measured results and what failed.
6. **Nothing is thrown away.** Each lab's code becomes a component of the capstone (the task monitor, the drivers, the protocol parser). Keep them in one repository and tag each lab.

A lab package is therefore four things: spec (pre-lab), demonstration, report, code. UTEP's ESP-IDF lab course (see below) grades exactly those four pieces (pre-lab 5%, demonstration 30%, report 25%, code 20%), so you can score yourself on them if you want a finer scale than the weekly 0-3.

## Compatibility audit and diagnostics

Every lab was checked against the v6.1 API pages, the v6.1 headers and Kconfig files at the `v6.1` source tag, the v6.1 example READMEs, the v6.0 migration guides, the WROVER-E datasheet (v2.4), the ESP32 Series Datasheet (v5.3), and the Freenove tutorial, sketches and datasheet folder. **None of the labs was compiled or run on hardware**; the audit is documentation-based. Diagnostics use compiler severity:

- `error:` the assignment will not work as written on the Freenove WROVER-E board with v6.1 without the stated change or extra hardware.
- `warning:` it will compile, but it can waste hours or damage the setup if you skip the note.
- `note:` a fact worth knowing.

**Golden rule:** if a function is not on a v6.1 API reference page or in a v6.1 header, do not use it. Tutorials and blog posts written for older ESP-IDF releases use drivers that **no longer exist in v6.x**. The [v6.0 peripherals migration guide](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/migration-guides/release-6.x/6.0/peripherals.html) says the legacy ADC (`driver/adc.h`), timer-group (`driver/timer.h`), PCNT (`driver/pcnt.h`), RMT (`driver/rmt.h`), MCPWM, I2S, DAC and sigma-delta drivers were completely removed in v6.0, and the legacy I2C driver (`driver/i2c.h`) is end-of-life and scheduled for removal in v7.0. Use the new drivers named in each lab.

> **error:** copy-pasting code from an older tutorial (`adc1_get_raw`, `esp_adc_cal_*`, `timer_init`, `pcnt_config_t`, `i2c_driver_install`, `rmt_config`) will fail to build or build against a driver on its way out. `[-Wlegacy-driver]`

> **warning:** since v6.0, public driver headers no longer include FreeRTOS headers, so code that relied on that must `#include "freertos/FreeRTOS.h"`, `"freertos/task.h"`, `"freertos/queue.h"` and so on explicitly. The same guide says the old `driver` component no longer re-exports the `esp_driver_*` components: list the ones you use in `REQUIRES`/`PRIV_REQUIRES` (for example `esp_driver_gpio`, `esp_driver_uart`, `esp_driver_i2c`, `esp_adc`). `[-Wmissing-includes]`

> **warning:** the v6.0 system migration guide says the default C library changed from Newlib to Picolibc (a Newlib fork with a rewritten stdio). If `printf` formatting or a third-party library behaves differently from what a tutorial shows, check that guide before blaming your code. The v6.0 tools guide also says the minimum Python version is 3.10. `[-Wsdk-version]`

> **note:** the v6.0 system guide says most FreeRTOS functions now live in flash by default instead of IRAM to save IRAM, while the ISR-callable functions stay in IRAM unless `CONFIG_FREERTOS_PLACE_ISR_FUNCTIONS_INTO_FLASH` is set. That matters for the interrupt-latency measurements in Stages 1 and 2.

### Freenove WROVER-E pin rules (read before wiring anything)

These come from the WROVER-E datasheet (pin table and boot-configuration chapter), the Freenove pinout picture (`ESP32_Pinout_V3.0.png`) and tutorial, and the v6.1 [GPIO reference](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/gpio.html). The kit's `Datasheet` folder contains the datasheets for the older WROVER and WROVER-B modules but **not** the -E, so use the link above, not the folder.

> **note:** the module brings out 24 GPIOs: 0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36 and 39. GPIO6-GPIO11 are connected to the module's SPI flash and are not led out (datasheet Table 3, footnote). GPIO16 and GPIO17 do not appear in the module's pin table either, and Freenove's tutorial says they connect the integrated PSRAM. Never configure any of these eight pins. `[-Wflash-psram-pins]`

> **warning:** GPIO34, 35, 36 and 39 are input-only. The v6.1 GPIO page says GPIO34-39 have no software-enabled pull-up or pull-down, so a button on one of them needs an external resistor. `[-Winput-only-pins]`

> **warning:** GPIO0, 2, 5, 12 (MTDI) and 15 (MTDO) are strapping pins. Datasheet Table 4 gives their defaults (GPIO0 pull-up, GPIO2 pull-down, GPIO5 pull-up, MTDI pull-down, MTDO pull-up). GPIO0 held low at reset enters the serial bootloader. MTDI selects the VDD_SDIO voltage at reset (1 selects the 1.8 V regulator unless an eFuse overrides it), and the WROVER-E table lists the module's VDD_SDIO as 3.3 V, so do not let anything drive GPIO12 high at reset. MTDO controls boot-log printing on U0TXD. On the Freenove board GPIO0 is the BOOT button and GPIO2 drives the on-board LED. `[-Wstrapping-pin]`

> **warning:** Freenove's Arduino sketches freely use strapping and JTAG pins (the LED bar on GPIO15, 2, 0, 4, 5...; the RGB LED on 4, 2, 15; the 74HC595 on 12, 13, 14; I2C on 13 and 14). **Do not copy Freenove's pin numbers.** Lab 0 is where you re-plan the whole course on safe pins. An LED that is off does not load a pin, but a reversed LED or a short to ground at reset can. `[-Wstrapping-pin]`

> **note:** GPIO1 and GPIO3 are UART0, the console your USB link uses (Freenove says the board downloads code through a CH340 USB-serial chip). Keep them free for programming and logging unless Lab 4a says otherwise. The JTAG pins are GPIO12 (TDI), 13 (TCK), 14 (TMS) and 15 (TDO) (Stage 5).

> **warning:** the board's camera connector shares pins with the headers (Freenove's table: GPIO4, 5, 18, 19, 21, 22, 23, 25, 26, 27, 34, 35, 36, 39). With the camera unplugged those pins are free. Keep the camera in its bag for this course unless you do the optional camera stretch. `[-Wcamera-pins]`

> **note:** the microSD slot uses the ESP32's fixed SDMMC pins (Freenove marks them "do not modify"): GPIO14 CLK, GPIO15 CMD and GPIO2 D0. The v6.1 `sdmmc` example README warns that on the ESP32 these pins cannot be changed and that GPIO2 pulled low enters download mode. Treat SD logging as the optional stretch in Stage 5 and not part of the core labs.

> **note:** ADC1 channels are on GPIO32-GPIO39 (GPIO36, 39, 34, 35, 32 and 33 are led out); ADC2 channels are on GPIO0, 2, 4, 12, 13, 14, 15, 25, 26 and 27 (WROVER-E datasheet Table 3). Freenove's Arduino ADC projects use ADC2 pins (GPIO4 and GPIO13). In v6.1 the oneshot driver documents that ADC2 is shared with Wi-Fi and that `adc_oneshot_read()` arbitrates between the two, but the labs still use ADC1 pins so they never depend on that.

> **note:** the module has PSRAM (the WROVER-E series is sold with 8 MB of Quad SPI PSRAM, and 4, 8 or 16 MB of flash; the older 2 MB PSRAM variants are marked EOL in the datasheet). **Read the part number on your module's shield** (for example ESP32-WROVER-E-N4R8 means 4 MB flash and 8 MB PSRAM) and the bootloader log from `idf.py monitor` to confirm your flash size. The v6.1 [external RAM guide](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/external-ram.html) says the ESP32 can map only up to 4 MB of PSRAM into its address space, and the [Himem API](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/himem.html) exists for banking in memory beyond that. `[-Wmodule-variant]`

> **note:** the datasheet lists the chip as ESP32-D0WD-V3 (chip revision 3.x). The v6.1 external RAM guide says the ESP32 revision v1.0 PSRAM cache bug (workaround `CONFIG_SPIRAM_CACHE_WORKAROUND`) is resolved in revision v3.0, so you should not need that workaround.

## Materials list

**Hardware (required)**
- The **Freenove Ultimate Starter Kit for ESP32** with its ESP32-WROVER-E board, GPIO extension board and USB cable. From the kit's parts photo, the pieces the labs use are: breadboard and jumper wires; resistors (220 Ω, 1 kΩ, 10 kΩ); capacitors (0.1 µF and 10 µF); potentiometers; push buttons; LEDs, an RGB LED, a 10-segment LED bar graph and an 8-LED WS2812-style RGB module; active and passive buzzers; photoresistor and thermistor; NPN and PNP transistors; DHT11; HC-SR04 ultrasonic module; MPU6050 accelerometer/gyro module; LCD1602 with I2C backpack; 74HC595 shift registers; 7-segment and 4-digit 7-segment displays; LED matrix; 4x4 keypad; IR receiver; PIR motion sensor; servo, DC motor with L293D, relay and stepper with ULN2003 driver; microSD card and reader; camera.
- A multimeter, needed for the calibration and electronics labs (not in the kit).
- A smartphone with the free [nRF Connect for Mobile](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-mobile) app (Android and iOS) for the BLE labs.

**Hardware (strongly recommended)**
- An 8-channel 24 MHz USB logic analyzer with the free PulseView software; SparkFun's [tutorial](https://learn.sparkfun.com/tutorials/using-the-usb-logic-analyzer-with-sigrok-pulseview) shows it decoding UART, I2C and SPI. SparkFun's own listing is marked retired, but any sigrok-compatible 8-channel 24 MHz analyzer follows the same tutorial. Every lab has a software-only fallback, but the analyzer is what lets you prove timing.
- A **3.3 V** USB-to-UART adapter (optional now: the board's own CH340 link plus an on-board UART1-to-UART2 loopback covers Lab 4a, see below).

**Hardware (optional)**
- A second ESP32 board, required for the NimBLE `throughput_app` benchmark (its README says two boards are needed) and useful as a BLE or UART peer. The kit contains only one.
- A JTAG adapter such as ESP-Prog for Stage 5. The [v6.1 JTAG page](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/jtag-debugging/index.html) says ESP-WROVER-KIT has JTAG built in; the Freenove board is not that board, so JTAG needs an external adapter.
- A CAN transceiver and a second CAN node, only if you want to try TWAI in Stage 4. The kit has no transceiver.
- A 9 V battery for the L293D motor assignment (Freenove's component list says to supply it yourself).
- An op amp for the optional half of Lab 3c. The kit has none.

**Software**
- ESP-IDF v6.1 via the [v6.1 Get Started guide](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/get-started/index.html), which installs through the ESP-IDF Installation Manager (EIM). Optionally use VS Code with the Espressif [IDF extension](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/index.html) that the Get Started page links. Python 3.10 or newer is required by v6.x.
- The kit's `CH340` folder holds the USB-serial drivers if your OS does not recognize the board.
- Git (one repository for all your labs), Python for the OSTEP simulators and for PC-side test scripts (the OSTEP README shows `python ./scheduler.py ...` without stating a version), and PulseView.

**Books and references**
- Free: [OSTEP](https://pages.cs.wisc.edu/~remzi/OSTEP/), [Mastering the FreeRTOS Real Time Kernel](https://en.freertos.org/Documentation/02-Kernel/07-Books-and-manual/01-RTOS_book), [The Scientist and Engineer's Guide to DSP](https://www.dspguide.com/pdfbook.htm), [Kalman and Bayesian Filters in Python](https://github.com/rlabbe/Kalman-and-Bayesian-Filters-in-Python) (free, as Jupyter notebooks) and, as an optional companion text, [Lee and Seshia's *Introduction to Embedded Systems*](https://ptolemy.org/books/leeseshia/download.html) (2nd ed., free PDF).
- Paid: [*Practical Electronics for Inventors*, 4th ed.](https://www.adafruit.com/product/1261) (Ch. 2, 6, 8 and 12 only) and Elecia White's [*Making Embedded Systems*, 2nd ed.](https://www.oreilly.com/library/view/making-embedded-systems/9781098151539/).
- Reference: the [ESP32-WROVER-E datasheet](https://documentation.espressif.com/esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf), the [ESP32 Series Datasheet](https://documentation.espressif.com/esp32_datasheet_en.pdf) (ADC characteristics, DC limits, peripherals) and the [Technical Reference Manual](https://documentation.espressif.com/esp32_technical_reference_manual_en.pdf).
- Kit documents (in your kit folder): `C/C_Tutorial.pdf` (circuits and component knowledge), `ESP32_Pinout_V3.0.png`, and the `Datasheet` folder (MPU-6050 product specification and register map, 74HC595, L293D, LCD1602, PCF8574, PCF8591).

**Other**
- A paper lab notebook and a fixed weekly calendar block (see below).

## How this course runs (mimicking a college course)

The structure is borrowed from real courses:

- **UTEP, ECE 4154/5190 "Laboratory for Microprocessor Systems II" (Fall 2025)** has students use ESP-IDF, with a [syllabus](https://digitalmeasures.utep.edu/ai/mbarua/schteach/Syllabus-ECE4154_5190_6190-Fall_2025CRN148511502817694-2.pdf) whose lab sequence runs LED lightshow, LED controller, interrupt system, introduction to FreeRTOS, semaphores, queues, GPIO interrupts with queues, ADC and PWM (LEDC), DAC, then Wi-Fi and IoT labs (servo control, ADC monitor), with a final project. Its labs are graded as pre-lab, demonstration, report and code, which is where this course's lab package comes from. It uses a different ESP32 module from yours, so I borrowed only the lab sequence and grading.
- **UT Dallas, EE/CE 4370 Embedded Systems (Spring 2024)** has a [syllabus](https://dox.utdallas.edu/syl138485) with weekly labs on GPIO and switches, timer and switch interrupts on the ESP32, PWM, finite state machines, ADC and DAC, wired protocols (UART, SPI, I2C), Wi-Fi and Bluetooth, and an RTOS lab, plus two exams (50% together), labs (25%) and homework (20%).
- **University of Bologna, Embedded Systems and IoT (2025/26)** has a [course page](https://www.unibo.it/en/study/course-units-transferable-skills-moocs/course-unit-catalogue/course-unit/2025/526556) that uses an ESP32 with FreeRTOS as its SoC/IoT platform and assesses three small projects.
- **Bucknell's [Internet of Things course](https://csci.courses.bucknell.edu/332/spring-2022-the-internet-of-things/)** uses ESP-IDF with FreeRTOS and ESP32 peripherals, meets twice a week, grades weekly progress reports on a 0-3 scale (the best 10 count), and ends in an individual project portfolio. I read it when I wrote the first version of this course; its server refused connections when I tried again today, so I could not re-read it, and its ESP-IDF release is older than yours. Borrow its structure only.

- **Weekly rhythm (about 8 hours):** two 80-minute "lecture" blocks (readings and videos), one 2-3 hour lab, and one problem-set session. Bucknell's classes run 80 minutes, which is where the block length comes from; the total is my suggestion.
- **Weekly progress report:** at the end of each week, score yourself 0-3 (0 = did not finish, 3 = lab meets its spec, report written and you can explain it unaided) and write three sentences in your lab notebook.
- **Study method:** the research says to test yourself and space it out. In [Dunlosky et al. (2013)](https://www.psychologicalscience.org/news/releases/which-study-strategies-make-the-grade.html), practice testing and distributed practice rated highest, while rereading, highlighting and summarizing rated low. So close the book and write out the answer before checking, and revisit earlier stages' material every week.
- **Office hours:** write down unanswered questions during the week and bring them to me in one batch.
- **Exams:** a closed-book midterm after Stage 2 and a final after Stage 4 (sample prompts are at the end of those stages). Grade yourself against the readings.
- **Pacing (suggested):** Stage 0 one week, Stage 1 three weeks, Stage 2 three weeks, Stage 3 four weeks, Stage 4 four weeks, Stage 5 two weeks, capstone three weeks.

## Arduino → ESP-IDF v6.1 cheat sheet

Each lab repeats the rows it needs. All names below were checked against v6.1 headers or doc pages.

| Arduino | ESP-IDF v6.1 | Notes |
|---|---|---|
| `setup()` / `loop()` | `app_main()` plus FreeRTOS tasks (`xTaskCreatePinnedToCore`, `tskNO_AFFINITY`) | No hidden `loop()` task: you own the tasks. |
| `pinMode`, `digitalWrite`, `digitalRead` | `gpio_config()` with `gpio_config_t`, `gpio_set_level()`, `gpio_get_level()` | `driver/gpio.h`, component `esp_driver_gpio`. `gpio_config()` overwrites all of that pin's configuration. |
| `INPUT_PULLUP` | `gpio_set_pull_mode()` or the pull fields in `gpio_config_t` | Not available on GPIO34-39. |
| `attachInterrupt` | `gpio_install_isr_service()`, `gpio_isr_handler_add()`, `gpio_set_intr_type()` | v6.1 GPIO page: handlers need no `IRAM_ATTR` unless you pass `ESP_INTR_FLAG_IRAM`. |
| `delay(ms)` | `vTaskDelay(pdMS_TO_TICKS(ms))` | Yields the CPU; Arduino's `delay` in `loop()` hid that. |
| `delayMicroseconds` | `esp_rom_delay_us()` (`esp_rom_sys.h`) | Busy-waits; use only for microsecond-scale protocol timing. |
| `millis()`, `micros()` | `esp_timer_get_time()` (microseconds, `esp_timer.h`), `xTaskGetTickCount()` | `esp_cpu_get_cycle_count()` (`esp_cpu.h`) for cycle-level timing. |
| `ledcAttach*`, `ledcWrite`, `analogWrite` | `ledc_timer_config()`, `ledc_channel_config()`, `ledc_set_duty()` + `ledc_update_duty()`; fades: `ledc_fade_func_install()`, `ledc_set_fade_with_time()`, `ledc_fade_start()` | `ledc_timer_set()` was removed in v6.0. |
| `ledcWriteTone` | `ledc_set_freq()` on a 50% duty channel | Frequency and duty resolution trade off. |
| `analogRead` | `adc_oneshot_new_unit()`, `adc_oneshot_config_channel()`, `adc_oneshot_read()`, `adc_cali_create_scheme_line_fitting()`, `adc_cali_raw_to_voltage()` | Headers under `esp_adc/`. Not callable from an ISR. |
| `Serial.print`, `Serial.printf` | `printf()` or `ESP_LOGI()` / `ESP_LOGE()` (`esp_log.h`) | Output goes to UART0 by default. |
| `Serial.read` / `Serial.available` | `uart_driver_install()`, `uart_read_bytes()`, or the `esp_console` REPL (`esp_console_new_repl_uart()`, `esp_console_cmd_register()`, `esp_console_start_repl()`) | Installing a UART driver on UART0 takes input away from the console. |
| `Wire.begin/beginTransmission/requestFrom` | `i2c_new_master_bus()`, `i2c_master_bus_add_device()`, `i2c_master_transmit()`, `i2c_master_receive()`, `i2c_master_transmit_receive()`, `i2c_master_probe()`, `i2c_master_bus_reset()` | `driver/i2c_master.h`. Not the legacy `driver/i2c.h`. |
| `SPI.transfer`, `shiftOut` | `spi_bus_initialize()`, `spi_bus_add_device()`, `spi_device_transmit()` / `spi_device_polling_transmit()` | Host names `SPI2_HOST`, `SPI3_HOST`; `SPI_DMA_CH_AUTO`. |
| `pulseIn` | GPIO interrupt on both edges plus `esp_timer_get_time()`, or RMT receive (`rmt_new_rx_channel()`) | See Lab 2c. |
| `hw_timer_t`, `timerBegin` | `gptimer_new_timer()`, `gptimer_register_event_callbacks()`, `gptimer_enable()`, `gptimer_start()`; or `esp_timer_create()` for software timers | `driver/gptimer.h`. The legacy timer-group driver is gone. |
| `touchRead` | `touch_sensor_new_controller()`, `touch_sensor_new_channel()`, `touch_channel_read_data()` | `driver/touch_sens.h`. The v6.1 `touch_sens_basic` example lists ESP32. |
| BLE library (`BLEDevice`...) | NimBLE or Bluedroid host stack | Lab 4b. |
| `WiFi.begin`, `WiFiClient`, `WiFiServer` | `esp_wifi`, `esp_netif`, `esp_event`, and BSD sockets | Lab 4e. |
| `SD_MMC` | `esp_vfs_fat_sdmmc_mount()` and then `fopen()`/`fprintf()` | Stage 5 stretch. |
| `IRremote`-style libraries | RMT receive (`rmt_new_rx_channel()`, `rmt_receive()`) | Lab 4f. |

## Stage 0: Foundations and orientation

**Readings**
- *Practical Electronics for Inventors*, [Ch. 2 Theory](https://www.adafruit.com/product/1261): the circuit fundamentals (Ohm's law, voltage dividers, RC behavior) that every GPIO, pull-up and ADC input depends on. Read only what you can't already do on paper.
- *Practical Electronics for Inventors*, [Ch. 12 Digital Electronics](https://www.adafruit.com/product/1261), for logic levels and digital signaling only. This matters as soon as you wire a 3.3 V ESP32 to 5 V parts.
- [ESP32-WROVER-E datasheet](https://documentation.espressif.com/esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf): module pin table (Table 3), strapping pins (Table 4 and the boot-configuration chapter) and the part-number table. Then the DC characteristics in the [ESP32 Series Datasheet](https://documentation.espressif.com/esp32_datasheet_en.pdf).
- Skim the [Technical Reference Manual](https://documentation.espressif.com/esp32_technical_reference_manual_en.pdf) (System and Memory, Interrupt Matrix) and come back to it as a reference, not cover to cover.
- Freenove `C_Tutorial.pdf`, "Preface": the board description, the GPIO tables and the strapping, flash and camera pin notes.

**Labs**
- **(Prof. Claude)** Lab 0: build a pin budget for the whole course. List every GPIO you will use across Stages 1-5 (a pulse output and input, ADC inputs, I2C bus, SPI bus, second UART, the LED bar, buzzer and button, test points), assign each a pin, and check every assignment against the pin rules above, the datasheet's pin tables and the Freenove pinout picture. Justify each choice in a sentence, and mark every place where you deliberately moved away from the pin Freenove's Arduino sketch used.
- **(Prof. Claude)** Paper lab: an ESP32 input pin must never see more than its datasheet limit. The ESP32 Series Datasheet's DC characteristics table gives VIH as 0.75 × VDD up to VDD + 0.3 V. Freenove's tutorial says the HC-SR04 runs at 5 V and the kit sketch reads its Echo pin straight on a GPIO. I could not read the tutorial's circuit diagram and the kit has no HC-SR04 datasheet, so check an HC-SR04 datasheet for the Echo output level. Pick a resistor pair from the kit (220 Ω, 1 kΩ, 10 kΩ, in series or parallel) that divides a **5 V** signal down to a safe level, calculate the output, then build it with the 5 V pin on the board header and check the real output with your multimeter **before** connecting it to any GPIO.

**Checkpoint:** explain, unaided, what a strapping pin is, why you should not tie one to an arbitrary sensor, and why GPIO6-11 and GPIO16-17 are off limits on this module.

## Stage 1: OS foundations, mapped onto the chip

**Readings**
- OSTEP: [Introduction](https://pages.cs.wisc.edu/~remzi/OSTEP/intro.pdf), [Processes](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf), [Direct Execution](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-mechanisms.pdf) (traps, interrupts and context switches are what FreeRTOS does on the ESP32) and [CPU Scheduling](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched.pdf).
- [Mastering the FreeRTOS Real Time Kernel](https://en.freertos.org/Documentation/02-Kernel/07-Books-and-manual/01-RTOS_book) (free), covering tasks, queues and software timers.
- [FreeRTOS Overview](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/freertos.html) and [FreeRTOS (IDF)](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/freertos_idf.html): the v6.1 pages describe ESP-IDF FreeRTOS as based on Vanilla FreeRTOS v10.5.1, modified for dual-core SMP.
- [Application Startup Flow](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/startup.html): boot to scheduler start to `app_main`, which makes sense once you know what a task and a scheduler are. It also says PSRAM is enabled during the port-initialization stage when configured.
- [Memory Types](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/memory-types.html) and [External RAM](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/external-ram.html) with OSTEP [Address Spaces](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf) and [Free Space Management](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-freespace.pdf). Read these for contrast: the ESP32 has no per-process address spaces, and the heap, the PSRAM and the DMA-capable buffers are yours to manage. This is where the WROVER-E's PSRAM matters.

**Lectures and videos**
- Digi-Key's *Introduction to RTOS* by Shawn Hymel ([playlist listing](https://videohighlight.com/playlist/PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz)). Use it as lecture material; you can skip Part 2 (a getting-started task demo) and treat its challenges as optional. This stage covers [Part 1, what an RTOS is](https://www.digikey.com/es/videos/d/digi-key-electronics/introduction-to-rtos-part-1-what-is-a-real-time-operating-system-rtos), [Part 3, task scheduling](https://www.youtube.com/watch?v=95yUbClyf3E) and [Part 4, memory management](https://www.youtube.com/watch?v=Qske3yZRW5I). (The YouTube links are built from the video IDs in the playlist listing.)

> **warning:** the series is written for the Arduino framework (`setup()`, `loop()`, `Serial`, `analogRead`) on an Adafruit Feather HUZZAH32. That board uses a different ESP32 module from yours, but it is still a dual-core ESP32, so the FreeRTOS behavior carries over. Its FreeRTOS calls are the same ones you will use in ESP-IDF, but the Arduino calls do not exist in ESP-IDF, so use the cheat sheet above and replace `Serial` with `printf` or the ESP-IDF logging macros in anything you port. Do not assume Arduino's `loop()` core assignment carries over to `app_main`. `[-Wsdk-version]`

**Labs and problem sets**
- OSTEP homework simulators ([repository](https://github.com/remzi-arpacidusseau/ostep-homework/)): `cpu-intro`, `cpu-sched` and `vm-freespace`. Work the problems by hand first, then check with the simulator's `-c` flag. If `./scheduler.py` will not run directly, the README's alternative is `python ./scheduler.py ...`.

**Lab 1a (Freenove Project 3.1 Flowing Light, Project 4.2 Meteor Flowing Light): a flowing light that is also a scheduler experiment**
- *Freenove source:* `C/Sketches/Sketch_03.1_FlowingLight` and `Sketch_04.2_FlowingLight2` (10-LED bar graph through 220 Ω resistors; 4.2 fades the LEDs with PWM). Read the circuit and the behavior, not the code.
- *Build:* the same flowing-light patterns twice. Version A is one super-loop task with delays. Version B is one FreeRTOS task per pattern (or per LED group) at different priorities and on different cores, so you can watch what the scheduler does to the pattern when the tasks compete. Then add a deliberately CPU-hungry task and report what happens to each version's timing.
- *Swap list:* `pinMode`/`digitalWrite` → `gpio_config()`/`gpio_set_level()`; `delay` → `vTaskDelay(pdMS_TO_TICKS())`; `ledcAttachChannel`/`ledcWrite` (4.2) → `ledc_timer_config()`, `ledc_channel_config()`, `ledc_set_duty()`/`ledc_update_duty()`; tasks → `xTaskCreatePinnedToCore()`.
- *Acceptance:* per-LED step period measured (timestamps with `esp_timer_get_time()`, or the logic analyzer) in both versions, with the jitter stated in numbers, under idle and under load.
- *Break it:* give the hungry task a higher priority than the pattern tasks and then the same priority; explain both results with the OSTEP scheduling vocabulary.

> **warning:** the bar graph in Freenove's sketch sits on GPIO15, 2, 0, 4, 5, 18, 19, 21, 22 and 23, including strapping pins. Re-plan it in Lab 0 onto safe pins (you have 10 LEDs to place). `[-Wstrapping-pin]`

> **warning:** Project 4.2 fades all 10 LEDs with PWM at once. The v6.1 LEDC page says the ESP32 has two groups of LEDC channels, one high-speed and one low-speed, with 8 channels each, so 10 channels need both groups (`LEDC_HIGH_SPEED_MODE` and `LEDC_LOW_SPEED_MODE`). Low-speed timer changes must be triggered explicitly by software, while high-speed mode changes over glitch-free. `[-Wledc-groups]`

**(Prof. Claude) Lab 1: build a reusable task and heap monitor** that reports, at a configurable interval, each task's state, priority and stack high-water mark, the CPU time split between tasks and between the two cores, and the free and minimum-ever free heap, **separately for internal RAM and PSRAM**. Then use it to answer three questions with data: (1) how does CPU load divide across the two cores when you pin busy tasks versus leave them unpinned; (2) what stack size does a given task actually need, and how much margin did you leave; (3) how long does it take from a timer alarm to a waiting task running, under idle and heavy load. Measure (3) on the logic analyzer if you own one; otherwise timestamp the alarm callback and the task wake-up with `esp_timer_get_time()` or `esp_cpu_get_cycle_count()` and say how much error the measurement itself adds. Acceptance criteria include the monitor's own measured overhead. As a kit tie-in, drive the LED bar from Lab 1a as a live per-core load meter. This module becomes your debugging tool for every later lab.

> **note:** the [FreeRTOS (IDF) page](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/freertos_idf.html) documents `xTaskCreatePinnedToCore`, `tskNO_AFFINITY`, `vTaskGetInfo`, `uxTaskGetStackHighWaterMark`, `uxTaskGetSystemState`, `vTaskList`, `vTaskGetRunTimeStats` and `ulTaskGetIdleRunTimeCounter`. For the heap, `esp_get_free_heap_size` and `esp_get_minimum_free_heap_size` are on the v6.1 [Miscellaneous System APIs page](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/misc_system_api.html), and `heap_caps_get_free_size`, `heap_caps_get_minimum_free_size` and `heap_caps_get_largest_free_block` with `MALLOC_CAP_INTERNAL` and `MALLOC_CAP_SPIRAM` are on the [heap allocation page](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/mem_alloc.html). The current core is reported by `xPortGetCoreID()` (in the Xtensa `portmacro.h` of the v6.1 tree) and `esp_cpu_get_core_id()` (in `esp_cpu.h`).

> **warning:** the trace-facility and run-time-stats functions only exist if the matching options are enabled. The v6.1 FreeRTOS Kconfig names them `CONFIG_FREERTOS_USE_TRACE_FACILITY`, `CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS` (which `vTaskList()` and `vTaskGetRunTimeStats()` need) and `CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS` (which selects the other two). The run-time-stats clock is a choice between `CONFIG_FREERTOS_RUN_TIME_STATS_USING_ESP_TIMER` (the default) and `CONFIG_FREERTOS_RUN_TIME_STATS_USING_CPU_CLK`, and `CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID` adds a core column to `vTaskList`. The FreeRTOS pages say `FreeRTOSConfig.h` is private and you configure FreeRTOS through menuconfig. `[-Wkconfig]`

**(Prof. Claude) Lab 1c (WROVER-E only): the PSRAM lab.** Allocate buffers from internal RAM and from PSRAM with `heap_caps_malloc(..., MALLOC_CAP_INTERNAL)` and `heap_caps_malloc(..., MALLOC_CAP_SPIRAM)`. Measure sequential and random read/write throughput for each at several sizes (the v6.1 External RAM guide says the cache hides PSRAM latency for small working sets and that accessing large chunks, over 32 KB, can fall back to the PSRAM's own speed; test that claim). Then (a) run a flash write (an NVS commit) while a task walks a PSRAM buffer and show what happens, since the guide says external RAM becomes inaccessible when the flash cache is disabled; (b) place a task stack in PSRAM and report what happens when that task is running during the flash write (the guide says `xTaskCreate()` always uses internal memory for stacks and TCBs, and that stacks can go in external memory only if `CONFIG_FREERTOS_TASK_CREATE_ALLOW_EXT_MEM` is enabled and you create the task with `xTaskCreateStatic()` and a PSRAM-allocated stack buffer); (c) put a DMA buffer in PSRAM and see what the driver you use says (the Memory Types page says most peripheral DMA controllers want word-aligned buffers in internal DRAM, and recommends `MALLOC_CAP_DMA`). Acceptance criteria: a throughput table, and a one-paragraph rule for what is allowed to live in PSRAM. Extend your Lab 1 monitor with the PSRAM numbers.

> **note:** to use PSRAM you must enable it in menuconfig (`CONFIG_SPIRAM`; the guide's `CONFIG_SPIRAM_USE` options choose whether it joins the memory map, the capability allocator or plain `malloc`). Check the External RAM page for the exact option names in your menuconfig, and confirm your module's PSRAM voltage matches its flash voltage as the guide requires. `[-Wspiram-config]`

**Checkpoint:** draw the state diagram of a FreeRTOS task, and say which OSTEP concept each transition corresponds to.

## Stage 2: Concurrency and interrupts (where firmware bugs come from)

**Readings**
- OSTEP: [Concurrency and Threads](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-intro.pdf), [Locks](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf), [Condition Variables](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-cv.pdf), [Semaphores](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-sema.pdf) and [Concurrency Bugs](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-bugs.pdf).
- [FreeRTOS (IDF) (v6.1)](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/freertos_idf.html) and [FreeRTOS (Supplemental Features)](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/freertos_additions.html) for queues, semaphores, event groups, stream buffers and ring buffers, the practical counterparts of the chapters above.
- OSTEP [Multi-CPU Scheduling](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-multi.pdf), then the SMP sections of the FreeRTOS (IDF) page: task pinning across the two Xtensa cores, and spinlock critical sections (`portMUX_TYPE` with `taskENTER_CRITICAL(&spinlock)` and `taskEXIT_CRITICAL(&spinlock)`) in place of "disable interrupts".
- [Interrupt Allocation](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/intr_alloc.html) (shared interrupts, IRAM-safe handlers).
- OSTEP [Event-based Concurrency](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-events.pdf), then the [Event Loop Library](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_event.html) (default and custom event loops, `esp_event_handler_register`, `esp_event_post`).
- The v6.1 [PCNT](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/pcnt.html), [LEDC](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/ledc.html), [GPTimer](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/gptimer.html) and [ESP Timer](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/esp_timer.html) pages, for the labs below.

**Lectures and videos**
- Digi-Key *Introduction to RTOS*, Parts 5-12: [queues](https://www.youtube.com/watch?v=pHJ3lxOoWeI), [mutexes](https://www.youtube.com/watch?v=I55auRpbiTs), [semaphores](https://www.youtube.com/watch?v=5JcMtbA9QEE), [software timers](https://www.youtube.com/watch?v=b1f1Iex0Tso), [hardware interrupts](https://www.youtube.com/watch?v=qsflCf6ahXU) (with its [solution](https://www.digikey.com/es/maker/projects/introduction-to-rtos-solution-to-part-9-hardware-interrupts/3ae7a68462584e1eb408e1638002e9ed)), [deadlock and starvation](https://www.youtube.com/watch?v=hRsWi4HIENc), [priority inversion](https://www.youtube.com/watch?v=C2xKhxROmhA) and [multicore systems](https://www.digikey.com/en/videos/d/digi-key-electronics/introduction-to-rtos-part-12-multicore-systems). Watch them as lectures; the labs below replace the per-video challenges.

> **error:** Part 9's solution uses Arduino-only calls (`timerBegin`, `timerAttachInterrupt`, `timerAlarmWrite`, `timerAlarmEnable`, `analogRead` and `Serial`). They do not exist in ESP-IDF. If you port it, rebuild the hardware timer with the GPTimer driver (`gptimer_new_timer`, `gptimer_register_event_callbacks`, `gptimer_enable`, `gptimer_start`; see the [timer_group/gptimer example](https://github.com/espressif/esp-idf/tree/v6.1/examples/peripherals/timer_group/gptimer) for reference), and do not copy the code. The old timer-group driver it would otherwise lead you to was removed in v6.0. `[-Wlegacy-driver]`

> **error:** Part 9's challenge calls the ADC from inside the timer ISR. The v6.1 [ADC oneshot page](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/adc/adc_oneshot.html) states that `adc_oneshot_read()` should **not** be called in an ISR context. Have the ISR give a semaphore or send a task notification and let a task read the ADC. `[-Wisr-safety]`

**Labs and problem sets**
- OSTEP homework simulators: `threads-intro`, `threads-locks`, `threads-cv`, `threads-sema`, `threads-bugs` and `cpu-sched-multi`.

**Lab 2a (Freenove Project 2.1 Button & LED, Project 7.1 Doorbell, Project 7.2 and 7.3 Alertor): interrupt-driven input and a timer-driven alarm**
- *Freenove source:* `Sketch_02.1_ButtonAndLed`, `Sketch_07.1_Doorbell`, `Sketch_07.2_Alertor`, `Sketch_07.3_Alertor` (button read by polling in `loop()`; active buzzer for the doorbell; passive buzzer swept between roughly 1500 and 2500 Hz with `ledcWriteTone`, and a version that toggles the buzzer pin from a hardware-timer interrupt).
- *Build:* a button-driven alarm with no polling. A GPIO interrupt (rising and falling edge) timestamps the edge and hands it to a task through a queue or task notification; the task debounces and starts or stops the alarm. The alarm sweeps the passive buzzer frequency from a periodic timer callback, not from a delay loop. Build it twice for the sweep: once with `esp_timer` (software timer) and once with a GPTimer alarm callback toggling the pin (7.3's approach), and compare their pitch jitter.
- *Swap list:* `attachInterrupt` → `gpio_install_isr_service()` + `gpio_isr_handler_add()`; `digitalRead` polling → queue/notification from the ISR (`xQueueSendFromISR`, `vTaskNotifyGiveFromISR`); `ledcWriteTone` → `ledc_set_freq()`; `timerBegin`/`timerAlarm` → `gptimer_*` or `esp_timer_create()` + `esp_timer_start_periodic()`; `IRAM_ATTR onTimer()` → a GPTimer alarm callback.
- *Acceptance:* zero missed presses in a 100-press test, measured bounce counts before and after debouncing, and a measured pitch-jitter figure for each timer method.
- *Break it:* run a flash write (NVS commit) during the alarm and see which timer method glitches. The `esp_timer` header says its callbacks run from the `esp_timer` task by default (`ESP_TIMER_TASK`) and offers an ISR dispatch method, and the Interrupt Allocation page covers IRAM-safe handlers.

> **warning:** Freenove's wiring puts the button on GPIO4 or GPIO13 and the buzzer on GPIO13, and the button reads LOW when pressed (`digitalRead == LOW`). If your Lab 0 budget moves them, keep the active-low logic or invert it deliberately. If you use GPIO34-39 for the button, add an external pull-up. `[-Winput-only-pins]`

**(Prof. Claude) Lab 2: build a pulse-train measurement instrument and find where it breaks.** Generate a pulse train with LEDC on one pin, jumper it to an input pin (the kit's jumper wires are enough), and measure its frequency and jitter two ways: (A) a GPIO interrupt that timestamps each edge into a queue or ring buffer, with a task computing frequency and jitter, and (B) PCNT counting over fixed gate windows. Sweep the frequency upward. Using sequence numbers or timestamp accounting, prove exactly where method A starts losing events, compare with B, and write down why (ISR latency, queue depth, what else the CPU is doing). Then run a flash operation, such as an NVS write, during the measurement and see whether your ISR path stalls; the Interrupt Allocation page covers IRAM-safe handlers (`ESP_INTR_FLAG_IRAM`). Acceptance criteria: a stated event-loss count at each frequency, and a defended frequency limit for each method.

**(Prof. Claude) Lab 2b:** reproduce a priority inversion on purpose, prove it with your Lab 1 monitor, fix it, and show the fix in the numbers.

**Lab 2c (Freenove Project 21.1 and 21.2 Ultrasonic Ranging): replacing `pulseIn`**
- *Freenove source:* `Sketch_21.1_Ultrasonic_Ranging` (HC-SR04: a 10 µs trigger pulse, then `pulseIn()` measures how long Echo stays high; distance = time × speed of sound / 2) and `Sketch_21.2` (the same through a library).
- *Build:* a ranging task that triggers the sensor and measures the Echo pulse without a blocking `pulseIn`. Implement it two ways: (A) a GPIO interrupt on both edges with `esp_timer_get_time()` timestamps, and (B) RMT receive (`rmt_new_rx_channel()` + `rmt_receive()`), which captures pulse widths in hardware. Include a timeout so a missing echo cannot hang the task, convert to centimeters, and compare both methods against a ruler at several distances.
- *Swap list:* `pulseIn` → ISR timestamps or RMT RX; `delayMicroseconds(10)` → `esp_rom_delay_us(10)`; `Serial.print` → `ESP_LOGI`; temperature compensation (the library's `setTemperature`) → your own speed-of-sound formula.
- *Acceptance:* accuracy and repeatability in a table (mean, standard deviation, error per distance), the maximum measured range with a timeout you derived, and the CPU cost of each method from your Lab 1 monitor.
- *Break it:* aim the sensor at open space (no echo), at a soft surface, and at an angled surface, and show your timeout and error handling cope.

> **warning:** Echo needs level shifting (your Stage 0 paper lab). Do not wire it straight to a GPIO unless you have confirmed its output level. `[-Wlogic-level]`

> **error:** do not use the v6.1 `timer_group/gptimer_capture_hc_sr04` example: its README lists only newer chips (ESP32-C5, C6, C61, H2, H21, H4, P4 and S31) and it relies on the event task matrix, which the ESP32 does not have. Use the two methods above. `[-Wmissing-hardware]`

> **warning:** PCNT counters are 16-bit at most, as the v6.1 PCNT page says ("16 bit at most, defined in the hardware"), so at high frequencies they overflow fast. Set `low_limit`/`high_limit` in `pcnt_unit_config_t`, set the `accum_count` flag so the driver accumulates across overflows, and/or add watch points with `pcnt_unit_add_watch_point()`. The glitch filter is set in nanoseconds with `pcnt_unit_set_glitch_filter()` (`max_glitch_ns`); the driver converts it to APB clock cycles and rejects values beyond the hardware limit with `ESP_ERR_INVALID_ARG`, so find the largest accepted value by experiment, and remember a filter longer than your pulse width will swallow the pulses. `[-Wcounter-overflow]`

> **warning:** the v6.1 [FreeRTOS (IDF) page](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/freertos_idf.html) describes dual-core SMP scheduling, so tasks pinned to different cores run truly in parallel and a classic priority-inversion demo can silently fail to reproduce. Pin all three demo tasks to the **same** core. `[-Wpriority-inversion-smp]`

> **note:** LEDC can drive PWM on any output GPIO. On the ESP32 it has 16 channels (8 high-speed and 8 low-speed), and frequency and duty resolution trade off: the v6.1 page's example is that a 5 kHz PWM can have a maximum duty resolution of 13 bits. The ESP32 has high-speed LEDC mode, which most other chips do not.

> **warning:** an input on GPIO34-GPIO39 has no software pull-up. The v6.1 GPIO page says that when you use `gpio_install_isr_service()` the per-pin handlers need no `IRAM_ATTR` unless you pass `ESP_INTR_FLAG_IRAM`, which is the flag that makes them survive flash operations. `[-Winput-only-pins]`

**Midterm (closed book):**
- Trace what happens from a GPIO edge to a task running, naming every OS concept involved.
- Why is "disable interrupts" not a valid critical section on the ESP32?
- Given a two-task deadlock in code you have not seen, find and fix it.
- When would you choose PCNT over a GPIO interrupt, and what limits each?
- Your ISR reads the ADC and your buzzer glitches whenever you save settings. Name both bugs and the fixes.
- Which of these may live in PSRAM on this board, and which may not: a task stack, a DMA buffer, an IRAM-safe ISR's data, a large lookup table?

## Stage 3: Sensor input to useful data

**Readings**
- OSTEP [I/O Devices](https://pages.cs.wisc.edu/~remzi/OSTEP/file-devices.pdf) (polling vs interrupts vs DMA).
- *Practical Electronics for Inventors*, [Ch. 6 Sensors](https://www.adafruit.com/product/1261) (how common sensor types work and what they output) and [Ch. 8 Operational Amplifiers](https://www.adafruit.com/product/1261) (conditioning a sensor signal to fit the ADC's limited input range; the ESP32 Series Datasheet gives the ADC's calibrated measurement range as 150 to 2450 mV at its highest attenuation setting, and says accuracy gets worse above that).
- Peripheral drivers: [GPIO](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/gpio.html), [ADC overview](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/adc/index.html), [ADC oneshot mode](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/adc/adc_oneshot.html) and [ADC calibration](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/adc/adc_calibration.html), [GPTimer](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/gptimer.html), [I2C](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/i2c.html) and [SPI master](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/spi_master.html). Calibrate before trusting millivolt values.
- [Non-Volatile Storage (NVS)](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/storage/nvs_flash.html), for persisting calibration data.
- Free signal-processing text for making sensor data useful: [The Scientist and Engineer's Guide to DSP](https://www.dspguide.com/pdfbook.htm), specifically [Ch. 3 ADC and DAC](http://www.dspguide.com/ch3.htm), [Ch. 15 Moving Average Filters](http://www.dspguide.com/ch15.htm) and [Ch. 19 Recursive Filters](http://www.dspguide.com/ch19.htm).
- Elecia White, [*Making Embedded Systems*, 2nd ed.](https://www.oreilly.com/library/view/making-embedded-systems/9781098151539/) (paid). The second edition adds sensor and data-handling chapters, and it is the best single companion for design patterns.
- For the capstone's angle estimation: [Kalman and Bayesian Filters in Python](https://github.com/rlabbe/Kalman-and-Bayesian-Filters-in-Python) (free), and the [Madgwick orientation filter](https://ahrs.readthedocs.io/en/latest/filters/madgwick.html) explanation, which cites Madgwick's 2010 report on IMU and MARG orientation filters.
- The kit's own datasheets for your sensors: `Datasheet/MPU-6050 Product Specification.pdf` and `MPU-6050 Register Map and Descriptions.pdf`, `74HC595.pdf`, `PCF8574_datasheet.pdf` and `LCD1602.pdf`.

**Lectures and tutorials**
- SparkFun's [I2C tutorial](https://learn.sparkfun.com/tutorials/i2c) (open-drain signaling, pull-ups, addressing, clock stretching) and [Serial Communication](https://learn.sparkfun.com/tutorials/serial-communication/all) (synchronous vs asynchronous, which also sets up Stage 4). I could only confirm text tutorials for this stage, not videos.

**Tools and reference examples (not assignments)**
- [i2c_tools](https://github.com/espressif/esp-idf/tree/v6.1/examples/peripherals/i2c/i2c_tools): a console with `i2cconfig`, `i2cdetect`, `i2cget`, `i2cset` and `i2cdump`. Use it to find your MPU6050 and explore its registers before you write code. Its README says to run `i2cconfig` first to set up the bus with your GPIOs (its example pins are GPIO18 and GPIO19), and that external pull-ups are recommended even though the driver enables the internal ones.
- [i2c_basic](https://github.com/espressif/esp-idf/tree/v6.1/examples/peripherals/i2c/i2c_basic) as a register-level reference for the new master driver. It is written for an **MPU9250**, not your MPU6050, and its SDA and SCL pins are set in menuconfig.
- The [adc/oneshot_read](https://github.com/espressif/esp-idf/tree/v6.1/examples/peripherals/adc/oneshot_read) example for API usage, and [timer_group/gptimer](https://github.com/espressif/esp-idf/tree/v6.1/examples/peripherals/timer_group/gptimer).

**Lab 3a (Freenove Project 26.1 MPU6050): a datasheet-driven I2C driver**
- *Freenove source:* `Sketch_26.1_Acceleration_Detection` (MPU6050 module at I2C address 0x68, or 0x69 with AD0 high, per Freenove's tutorial; the sketch uses the `MPU6050_tockn` library and prints acceleration and gyro values). Use the library for nothing.
- *Build:* write the driver from the kit's MPU-6050 register map, with no vendor library and no copied example. Cover the WHO_AM_I check (register 0x75; the datasheet says it holds the 6-bit I2C address in bits [6:1], so a healthy part reads back 0x68), waking the part from sleep (`PWR_MGMT_1`, 0x6B), configurable ranges (`GYRO_CONFIG` 0x1B, `ACCEL_CONFIG` 0x1C), sample rate (`SMPLRT_DIV` 0x19 and `CONFIG` 0x1A, where the datasheet gives sample rate = gyro output rate / (1 + SMPLRT_DIV)), burst reads from `ACCEL_XOUT_H` (0x3B), unit conversion, timeouts and bus recovery (a stuck SDA or SCL line). Verify every transaction against the logic analyzer's I2C decode if you own one, or against `i2c_tools` register dumps if not. Unit-test the register and scale-conversion logic off the device. Acceptance criteria: the driver survives unplugging the sensor mid-read and recovers when it is plugged back in, without a reboot.
- *Swap list:* `Wire.begin(SDA, SCL)` → `i2c_new_master_bus()` with `i2c_master_bus_config_t` (`i2c_port`, `sda_io_num`, `scl_io_num`, `clk_source`, `glitch_ignore_cnt`, `flags.enable_internal_pullup`); `Wire.beginTransmission`/`write`/`endTransmission` → `i2c_master_bus_add_device()` with `i2c_device_config_t` (`dev_addr_length`, `device_address`, `scl_speed_hz`, `scl_wait_us`) and `i2c_master_transmit()`; `Wire.requestFrom` → `i2c_master_receive()` or the register-read pattern `i2c_master_transmit_receive()`; "is anybody there?" → `i2c_master_probe()`; hung bus → `i2c_master_bus_reset()`.
- *Break it:* unplug SDA and SCL mid-burst, short SDA to ground briefly, and run the bus at the wrong speed; show your timeouts and `i2c_master_bus_reset()` recovery in the logs.

> **warning:** Freenove's sketch puts I2C on GPIO13 and GPIO14, which are the JTAG TCK and TMS pins (Stage 5), and they are ADC2/touch pins. Pick your own pins in Lab 0. `[-Wstrapping-pin]`

> **warning:** the v6.1 I2C page says the master operation functions are thread-safe through a bus semaphore and `i2c_new_master_bus()` can be called from any task, but other functions are not guaranteed to be thread-safe without a mutex. Single transactions are protected; a multi-transaction sequence is not. `[-Wthread-safety]`

> **warning:** the I2C master `enable_internal_pullup` flag is documented as not strong enough for high bus speeds, and the v6.1 I2C page says pull-ups usually range from 1 kΩ to 10 kΩ, that a higher frequency wants a smaller resistor (not below 1 kΩ), and that 2 kΩ to 5 kΩ is recommended. The kit's 1 kΩ and 10 kΩ resistors are the ends of that range, and you do not know whether your MPU6050 and LCD backpack already carry pull-ups, so measure the rise time and stay at 100 kHz until you have. The page also says the ESP32 controller does not support clock stretching when acting as an I2C slave, which does not affect this master-only lab. `[-Wpull-up]`

> **error:** the legacy `driver/i2c.h` is end-of-life in v6.0. Tutorials built on `i2c_driver_install`/`i2c_master_cmd_begin` use it; do not. `[-Wlegacy-driver]`

**Lab 3b (Freenove Project 9.1 ADC, 11.1 Soft Light, 12.1 Night Lamp, 13.1 Thermometer): an ADC characterization and calibration report**
- *Freenove source:* `Sketch_09.1_ADC_DAC`, `Sketch_11.1_SoftLight`, `Sketch_12.1_NightLamp`, `Sketch_13.1_Thermometer` (potentiometer, photoresistor and thermistor on an analog input with a divider; 12.1 maps light to LED brightness; 13.1 converts the thermistor voltage to °C).
- *Build:* (1) Sample a potentiometer on an **ADC1** pin against multimeter-measured voltages at 10 or more points. Compare raw counts, `adc_cali_raw_to_voltage()` millivolts and your own least-squares fit, and quantify noise as a function of oversampling and filtering (moving average, Ch. 15, and recursive, Ch. 19) and of sample rate. Store your calibration coefficients in NVS and show they survive a reset. (2) Then apply it: rebuild the Night Lamp (photoresistor controls LED brightness through LEDC) and the Thermometer (thermistor to °C, with the conversion maths written out from the thermistor's characteristics) with your calibrated, filtered readings instead of raw counts. The deliverable is a one-page report with an error-versus-input plot and the effective resolution you achieved.
- *Swap list:* `analogRead` → `adc_oneshot_read()` after `adc_oneshot_new_unit()` and `adc_oneshot_config_channel()` (`adc_oneshot_chan_cfg_t` with `ADC_ATTEN_DB_12` and `ADC_BITWIDTH_DEFAULT`); `analogReadResolution`/`map(...)` → your own scaling; raw counts to volts → `adc_cali_create_scheme_line_fitting()` (config `unit_id`, `atten`, `bitwidth`) then `adc_cali_raw_to_voltage()`, or `adc_oneshot_get_calibrated_result()`, which does both; `ledcWrite` → `ledc_set_duty()` + `ledc_update_duty()`; `map`/`constrain` → write them.
- *Break it:* feed the pin above the calibrated range (the pot's wiper reaches 3.3 V, above 2450 mV) and show what saturation does to your fit; then add a series resistor to the source and show the sampling error the datasheet warns about.

> **warning:** ADC pins. Put the analog input on an ADC1 pin (GPIO32-GPIO39; GPIO32 and GPIO33 are fully free, GPIO34-39 are input-only). Freenove's Arduino sketches use GPIO4 and GPIO13, which are ADC2. The v6.1 ADC page documents the oneshot and continuous modes, the attenuation levels `ADC_ATTEN_DB_0`, `ADC_ATTEN_DB_2_5`, `ADC_ATTEN_DB_6` and `ADC_ATTEN_DB_12`, and calibration; use the names above, not the legacy `adc1_config_width`/`adc1_get_raw`/`esp_adc_cal_*` names. The ESP32 calibration scheme in v6.1 is line fitting. `[-Wadc-pins]`

> **warning:** the ESP32 Series Datasheet says that by default there are about ±6% differences in measured results between chips and that its calibrated-range table covers 150-2450 mV at the highest attenuation (and 100-950 mV at the lowest), with accuracy worse above 2450 mV. Your report should show your own numbers, not the datasheet's. The Vref default is 1100 mV, with the real value between 1000 and 1200 mV depending on the chip (v6.1 ADC page). If the eFuse Vref is not burned on your board, the line-fitting config needs a `default_vref` estimate. `[-Wadc-accuracy]`

> **warning:** NVS needs a partition named `nvs` in your partition table. It is log-structured with wear leveling, but every commit is still a flash write, so persist calibration rarely, never at the sampling rate. `[-Wflash-wear]`

**Lab 3c (electronics): condition the signal in hardware.** Build an RC low-pass anti-alias filter (DSP Ch. 3) from the kit's resistors and capacitors (0.1 µF and 10 µF). Measure noise and bandwidth with and without it using your Lab 3b setup, and state which improved and by how much. The kit has no op amp, so for the Ch. 8 part build a buffer from the kit's NPN transistor (an emitter follower) and measure what it does to the source impedance the ADC sees, or use an op amp if you own a single-supply rail-to-rail one that works from 3.3 V.

**Lab 3d (Freenove Project 24.1 and 24.2 DHT11): a single-wire protocol under an RTOS**
- *Freenove source:* `Sketch_24.1_Temperature_and_Humidity_Sensor` (uses the `DHTesp` library) and `Sketch_24.2` (the same combined with the LCD); the module needs 3.3-5.5 V per Freenove's tutorial.
- *Build:* write the DHT11 read yourself from its timing diagram. Drive the data line low then release it, then time the high and low pulses of each of the 40 bits, verify the checksum, and expose the result through a task. Since one bit's difference is tens of microseconds, you must protect the bit loop from preemption and interrupts. Measure the checksum-failure rate with and without protection and with another busy task running.
- *Swap list:* `DHTesp` library → your own function; `pinMode(INPUT/OUTPUT)` toggling → `gpio_set_direction()` or open-drain via `gpio_od_enable()`; `delayMicroseconds` → `esp_rom_delay_us()`; the timing loop's clock → `esp_timer_get_time()`; critical section → a `portMUX_TYPE` spinlock with `taskENTER_CRITICAL(&spinlock)`/`taskEXIT_CRITICAL(&spinlock)`.
- *Acceptance:* checksum-failure rate (per 1000 reads) in three configurations, and the longest time interrupts were blocked by your critical section.
- *Break it:* shorten the line's pull-up, lengthen the wire, and disable the critical section; show how each changes the failure rate. Also compare with an RMT-receive-based decode if you want to avoid blocking at all.

> **note:** the v6.0 peripherals guide says the `rmt_tx_channel_config_t`'s `io_od_mode` member was removed; call `gpio_od_enable()` yourself for open-drain, and that the `io_loop_back` options were removed, so different driver objects may share one GPIO number (for example to bind RMT TX and RX to one pin for a 1-wire style bus).

**Lab 3e (Freenove Project 15.1 74HC595 Flowing Water Light, Project 16.1 and 16.2 7-Segment Displays): a real SPI master driver**
- *Freenove source:* `Sketch_15.1_FlowingLight02`, `Sketch_16.1_1_Digit_7-Segment_Display`, `Sketch_16.2_4_Digit_7-Segment_Display` (a 74HC595 shift register expands 3 GPIOs into 8 outputs with `shiftOut`; the 4-digit display multiplexes digits).
- *Build:* drive the 74HC595 with the ESP32's SPI peripheral instead of bit-banging. SPI mode 0, MSB first, with the 595's latch input (ST_CP, the pin that updates the outputs on a rising edge) wired to the SPI chip-select so the outputs update when the transaction ends. Then use it: (1) the LED bar flowing light, (2) the 4-digit display with a refresh task. Choose the clock rate from the 74HC595 datasheet's fMAX at your supply voltage (start well below it), and measure the refresh rate at which flicker becomes visible while your Lab 1 monitor's load generator runs.
- *Swap list:* `shiftOut`/`digitalWrite` bit-banging → `spi_bus_initialize()` (with `SPI_DMA_CH_AUTO` or `SPI_DMA_DISABLED`), `spi_bus_add_device()` with a `spi_device_interface_config_t` (clock speed, mode, `spics_io_num`), and `spi_device_transmit()` or `spi_device_polling_transmit()`; `SPI.transfer` → the same; LSB-first display data → the `SPI_DEVICE_TXBIT_LSBFIRST` flag if you want it.
- *Acceptance:* a scope or analyzer capture of clock, data and latch showing the transaction and the latch edge, the highest clock your wiring handles reliably, and the flicker threshold under load.
- *Break it:* run the bus faster than your wiring supports and show the corrupted pattern; run it from a task that gets starved and show the display glitch.

> **warning:** the v6.1 SPI master page lists default IOMUX pins for SPI2 (SCLK 14, MOSI 13, MISO 12, CS0 15) and SPI3 (SCLK 18, MOSI 23, MISO 19, CS0 5), and says the GPIO matrix limits the maximum clock to roughly 33-77% of the IOMUX maximum and adds about 25 ns of input delay on MISO. A 74HC595 needs neither the speed nor MISO, so any free pins work; prefer SPI3's IOMUX pins (18, 23, 5) so you stay away from the JTAG and SPI2/strapping pins. Do not copy Freenove's pins 12, 13 and 14. `[-Wspi-pins]`

> **note:** this lab fills the old SPI gap: the v6.1 `spi_master` examples are `hd_eeprom` (needs an external EEPROM) and `lcd` (needs an LCD), neither of which is in the kit, which is why Lab 3e uses the shift register.

**Lab 3f (Freenove Project 20.1 LCD1602): a second I2C device and a shared bus**
- *Freenove source:* `Sketch_20.1_Display_the_string_on_LCD1602` (an HD44780 LCD behind a PCF8574 I2C expander, default address 0x27 or 0x3F per Freenove's tutorial; the sketch uses `LiquidCrystal_I2C`).
- *Build:* write the PCF8574 + HD44780 4-bit driver yourself from `PCF8574_datasheet.pdf` and `LCD1602.pdf`, then put the LCD and the MPU6050 on the **same bus** with each driven by its own task. Display live angle data on the LCD while the IMU task samples at a fixed rate, and prove the two tasks cannot corrupt each other's transactions.
- *Swap list:* `LiquidCrystal_I2C` → your driver over `i2c_master_transmit()`; `lcd.print` → your own `lcd_puts()`; `lcd.setCursor` → the HD44780 set-DDRAM-address command; `delay(2)` for LCD timing → `vTaskDelay()` for ms waits and `esp_rom_delay_us()` for µs waits; shared bus → one `i2c_master_bus_handle_t` and two device handles.
- *Acceptance:* no corrupted display writes and no missed IMU samples over a one-hour run, proven with counters; bus utilization measured.
- *Break it:* run both tasks at the same priority, then make the LCD task slow (heavy `printf`s), and show whether the IMU rate suffers; explain with the Stage 2 concepts.

**Freenove kit extension assignments for Stage 3 (optional, 1-2 hours each, same rules)**
- **Joystick (Project 14.1):** two ADC axes plus a button. Read both axes on **ADC1** pins and debounce the button. Freenove uses GPIO13, 12 and 14, which are ADC2 and strapping/JTAG pins; re-plan them.
- **Touch lamp (Projects 10.1 and 10.2):** `touchRead` → `touch_sensor_new_controller()`, `touch_sensor_new_channel()`, `touch_channel_read_data()` (and `touch_sensor_enable()`, `touch_sensor_start_continuous_scanning()`). Touch pads are on GPIO4, 0, 2, 15, 13, 12, 14, 27, 33 and 32; note that several are strapping pins. The `touch_sens_basic` example is the reference.
- **RGB LED and WS2812 module (Projects 5.1, 5.2, 6.1, 6.2):** the common-anode RGB LED needs three LEDC channels (inverted logic, as Freenove's sketch does); the 8-LED WS2812 module needs a precisely timed single-wire stream, which is what the RMT peripheral is for (reference: [rmt/led_strip](https://github.com/espressif/esp-idf/tree/v6.1/examples/peripherals/rmt/led_strip), whose README lists the ESP32). Freenove puts the module's data pin on GPIO2, the strapping pin that also drives the on-board LED.
- **Servo (Projects 18.1 and 18.2):** `ESP32Servo` → LEDC at 50 Hz with a duty computed from the pulse width (take the pulse range from Freenove's tutorial and your servo). Freenove's pins are GPIO15 for the servo and GPIO34 for the pot.
- **Relay and motor (Projects 17.1 and 17.2):** L293D enable pin driven by LEDC PWM, direction by two GPIOs, a button and potentiometer as inputs. Freenove warns not to power the motor from the ESP32 and to use an external supply (a 9 V battery in its tutorial) with a shared ground.
- **Stepper (Project 19.1):** four GPIOs through the ULN2003 driver, with a step sequence generated from a timer callback; compare full-step and half-step sequences. (The `rmt/stepper_motor` example does not list the ESP32 and needs a STEP/DIR driver, so it is not a reference here.)
- **Matrix keypad (Projects 22.1 and 22.2):** write the row/column scan and debounce yourself (the `matrix_keyboard` example is ESP32-S2 only). Freenove uses GPIO14, 27, 26, 25 for rows and 13, 21, 22, 23 for columns.
- **PIR motion sensor (Project 25.1):** a digital input with an interrupt and a hold-off timer.
- **7-segment and LED matrix (Projects 16.1-16.3):** multiplex them from a refresh task, as in Lab 3e.

**Checkpoint:** why can you not just call the ADC as often as you like, and what is the minimum sample rate implied by the sampling theorem for the signal you chose?

## Stage 4: Communication (UART, Bluetooth, others)

**Readings**
- [UART driver (v6.1)](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/uart.html): driver installation, interrupts and pattern detection. Component dependency `esp_driver_uart`.
- OSTEP [Data Integrity and Protection](https://pages.cs.wisc.edu/~remzi/OSTEP/file-integrity.pdf), the theory behind checksums and framing for your own serial protocols.
- Bluetooth: [Adafruit's Introduction to BLE](https://learn.adafruit.com/introduction-to-bluetooth-low-energy/introduction) for GAP/GATT concepts, then the [v6.1 Bluetooth API overview](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/bluetooth/index.html), the [NimBLE API pages](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/bluetooth/nimble/index.html) and the [v6.1 bluetooth examples](https://github.com/espressif/esp-idf/tree/v6.1/examples/bluetooth). Bluedroid is the default and covers Classic plus BLE. NimBLE is BLE-only and lighter, so choose by whether Classic (SPP/A2DP) is needed.
- Wi-Fi and TCP/IP: the [Wi-Fi driver guide](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/wifi.html) and [lwIP TCP/IP](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/lwip.html). The lwIP page says the BSD sockets API is the supported interface, the Netconn API is enabled but unsupported, and the raw API is not supported.
- Other protocols: [TWAI (CAN)](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/twai.html) and the [RMT](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/peripherals/rmt.html) driver, which is how you decode timing-based protocols such as IR remote codes.

> **error:** TWAI has no lab because it cannot run on a bare board. The v6.1 page says the ESP32 has one TWAI controller and no integrated transceiver, so an external transceiver and a second CAN node are required (the kit has neither), and its controllers are not compatible with CAN FD frames (they interpret such frames as errors). The v6.1 driver API is `twai_new_node_onchip()` and friends, not the older install-driver calls. Treat it as reading unless you own that hardware. `[-Wmissing-hardware]`

**Lectures and tutorials**
- Nordic's free [Bluetooth LE Fundamentals](https://academy.nordicsemi.com/courses/bluetooth-low-energy-fundamentals/) course (about 8-10 hours, six lessons covering GAP roles, GATT, advertising, connections, data exchange, security and sniffing). Read Lessons 1-4 for concepts.

> **error:** the Nordic course's exercises require a Nordic nRF development kit. Skip the exercises; on an ESP32 you cannot complete them. `[-Wmissing-hardware]`

> **note:** Espressif's [BLE Introduction](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/ble/get-started/ble-introduction.html) guide now has a v6.1 page, and it tells you to flash `examples/bluetooth/ble_get_started/nimble/NimBLE_GATT_Server`. That project exists in the v6.1 tree (alongside `NimBLE_Beacon`, `NimBLE_Connection` and `NimBLE_Security`), and its README lists the ESP32 and says to test it with nRF Connect for Mobile. Use it for the phone workflow and the code walkthrough, not as your assignment.

**Reference examples (not assignments)**
- UART: the v6.1 [uart examples](https://github.com/espressif/esp-idf/tree/v6.1/examples/peripherals/uart) (`uart_echo`, `uart_events`, `uart_async_rxtxtasks`, `uart_select`, `uart_repl`...) show the driver API; skim them for usage and then build your own. `uart_echo`'s README says it uses GPIO4 (TX) and GPIO5 (RX) by default, configurable, and needs a 3.3 V USB-serial adapter.
- BLE (NimBLE, in [examples/bluetooth/nimble](https://github.com/espressif/esp-idf/tree/v6.1/examples/bluetooth/nimble)): `blehr` (heart-rate peripheral that sends notifications) and `bleprph` (peripheral with a GATT database and security), plus [ble_uart_service](https://github.com/espressif/esp-idf/tree/v6.1/examples/bluetooth/ble_uart_service), a ready-made UART-over-GATT peripheral with a write characteristic for RX and a notify characteristic for TX, that works with NimBLE or Bluedroid. Read it after you finish Lab 4b to compare designs.
- Classic Bluetooth (Bluedroid, in [examples/bluetooth/bluedroid/classic_bt](https://github.com/espressif/esp-idf/tree/v6.1/examples/bluetooth/bluedroid/classic_bt)): `bt_spp_acceptor`, `bt_spp_initiator`, `a2dp_sink_stream`.
- Wi-Fi and sockets: [wifi/getting_started/station](https://github.com/espressif/esp-idf/tree/v6.1/examples/wifi/getting_started/station) and `softAP`, and [protocols/sockets](https://github.com/espressif/esp-idf/tree/v6.1/examples/protocols/sockets) (`tcp_client`, `tcp_server`, `udp_client`, `udp_server`).
- RMT: [rmt/ir_nec_transceiver](https://github.com/espressif/esp-idf/tree/v6.1/examples/peripherals/rmt/ir_nec_transceiver) (its README lists the ESP32; the transmit half needs an IR LED, which the kit does not include).

**Lab 4a (Freenove Project 8.1 and 8.2 Serial Print, Serial Read and Write): a framed command and response protocol over UART**
- *Freenove source:* `Sketch_08.1_SerialPrinter` and `Sketch_08.2_SerialRW` (print text over the serial port and echo what you type).
- *Build:* design a **framed command and response protocol**. It needs a start marker, a length field, a command ID, a payload and an error-detecting checksum or CRC (use OSTEP's data-integrity chapter to choose), plus sequence numbers and ack/nack. Commands: get and set configuration, start and stop streaming, read status. Implement the parser as a state machine that you can unit-test off the device. Then write a PC-side script that fuzzes it with bit flips, dropped bytes, truncated frames and back-to-back frames. Acceptance criteria: the parser never crashes or hangs, resynchronizes within one frame after corruption, and you have measured throughput at the highest baud rate your wiring holds reliably.
- *Link options.* The board's CH340 bridge is wired to UART0 (GPIO1 and GPIO3), so the PC can talk to your protocol over the same USB cable you flash with. Then logs and the protocol share a UART, so either put the console elsewhere or disable it (`CONFIG_ESP_CONSOLE_*` options in menuconfig) and make the parser tolerate the bootloader text at reset; surviving that is a good fuzz case. For the fuzz suite you can also use **UART1 and UART2 of the same chip jumpered TX-to-RX and RX-to-TX** on safe pins from your Lab 0 budget, with the PC-side fuzzer on UART0 or no adapter at all. A 3.3 V USB-UART adapter simply gives you an independent PC link if you own one.
- *Swap list:* `Serial.begin(115200)` → `uart_param_config()` (baud, data bits, parity, stop bits, flow control in a `uart_config_t`), `uart_set_pin()`, `uart_driver_install()` (RX buffer size must exceed the hardware FIFO length); `Serial.print`/`write` → `uart_write_bytes()`, `uart_wait_tx_done()`; `Serial.read`/`available` → `uart_read_bytes()` with a tick timeout; text commands (8.2) → optionally the `esp_console` REPL; framing/idle detection → the driver's pattern detection (`uart_enable_pattern_det_baud_intr()`) or the event queue (`UART_DATA`, `UART_FIFO_OVF`, `UART_FRAME_ERR`...).
- *Break it:* inject the corruptions above with the fuzzer, and overflow the RX FIFO on purpose by blocking the reader task; show `UART_FIFO_OVF` handling and recovery.

> **warning:** use your own pins for UART1 and UART2, never GPIO1 and GPIO3 unless you are deliberately taking over the console. The v6.1 UART page notes that UART0 uses GPIO1 (TX) and GPIO3 (RX) via the IO_MUX, that UART1 and UART2 route through the GPIO matrix, and that GPIO9 and GPIO10 are used for flash on the ESP32, so do not leave UART1 on those pins (the WROVER-E does not even lead them out). Installing a UART driver on UART0 disables console input. `[-Wconsole-uart]`

> **warning:** the `bleprph` README says that NVS support is not yet integrated with bonding, so bonding is not persistent across reboot; do not design your acceptance criteria around a bonded reconnect based on that example. Its test instructions say any BLE scanner app can be used. `[-Wunverified-hardware-pairing]`

**Lab 4b (Freenove Project 27.2 BLE Data Passthrough, Project 27.3 Bluetooth Control LED): a custom BLE GATT service**
- *Freenove source:* `Sketch_27.2_BLE_USART` and `Sketch_27.3_BluetoothToLed` (a BLE UART service using the Nordic UART service UUIDs `6E400001-...`, `6E400002-...` for RX and `6E400003-...` for TX, and an LED controlled over Bluetooth).
- *Build:* a **custom BLE GATT service** for the same data as Lab 4a: a write characteristic for commands, a notify characteristic for data frames, a read characteristic for status, plus an LED-control characteristic as in Project 27.3. Choose NimBLE (BLE only) and say why. Read how the v6.1 NimBLE examples set up NVS before starting the stack, and explain what the stack uses it for. Handle subscribe and unsubscribe, disconnect and reconnect (restart advertising), and the notification size limit that the negotiated MTU sets. Use your own 128-bit UUIDs for the service. Verify everything in nRF Connect for Mobile. If you own a second board, benchmark against the [throughput_app](https://github.com/espressif/esp-idf/tree/v6.1/examples/bluetooth/nimble/throughput_app) example, whose README says it needs two boards and quotes roughly 340 kbps for notify, 200 kbps for read and 500 kbps for write (60 s test, MTU 512, 7.5 ms connection interval, DLE 251 bytes, 1M PHY), and explain any gap between its numbers and yours.
- *Swap list:* `BLEDevice::init`, `BLEServer`, `BLECharacteristic`, `BLE2902` (the Arduino BLE library) → NimBLE: enable it in menuconfig under Component config → Bluetooth (host selection), initialize NVS first (`nvs_flash_init()`), then define a GATT service table and access callbacks as in the `NimBLE_GATT_Server` and `bleprph` references; `characteristic->notify()` → a NimBLE notify call from your sender task; `Serial` bridge → your Lab 4a frame builder.
- *Break it:* disconnect during a notify stream, subscribe with the phone far away, change the MTU, and show the stack recovers and advertising restarts.

> **note:** `blecent` is a GATT client that only connects to a server advertising the Alert Notification service per its README, so do not assume it pairs with `bleprph` or `blehr`; skip it. The test scripts for `blehr`/`bleprph` that older tutorials mention are not needed: use nRF Connect for Mobile, which works on any platform.

**Lab 4c (Prof. Claude): make the protocol layer transport-agnostic.** The same command handler and frame builder should serve both UART bytes and BLE writes, with the transport as a thin adapter underneath. This is the shape of the packet protocols that device firmware and host software such as MyPod's EZAnything library exchange.

**Lab 4d (optional, Freenove Project 27.1 Bluetooth Passthrough): Classic Bluetooth SPP.** Serial over Bluetooth Classic needs the Bluedroid stack (NimBLE is BLE-only). Build a passthrough that reuses your Lab 4c frame layer over SPP and compare it with BLE for latency and throughput. Reference: `bt_spp_acceptor` in the v6.1 `classic_bt` examples. This is the lab that answers the final-exam question about when you are forced to use Bluedroid. The Bluedroid stack is larger, so watch your Lab 1 monitor's heap numbers.

**Lab 4e (Freenove Chapters 32 and 33, Wi-Fi working modes and TCP/IP): a third transport.**
- *Freenove source:* `Sketch_32.1_WiFi_Station`, `Sketch_32.2_WiFi_AP`, `Sketch_32.3_AP_Station`, and `Sketch_33.1_WiFiClient` / `Sketch_33.2_WiFiServer` (the Arduino `WiFi`, `WiFiClient` and `WiFiServer` classes). These are the C tutorial's chapter numbers; the kit's Python folder numbers the same projects 28 and 29.
- *Build:* connect in station mode, run a TCP server on a BSD socket, and plug it into the Lab 4c adapter as the third transport, so the same command handler now serves UART, BLE and TCP. Handle reconnects, dropped Wi-Fi and a client that sends garbage.
- *Swap list:* `WiFi.begin()` → `esp_netif_init()`, `esp_event_loop_create_default()`, `esp_wifi_init()`, `esp_wifi_set_config()`, `esp_wifi_start()` with handlers registered through `esp_event_handler_register()` (the event loop lab from Stage 2 pays off here); `WiFiServer`/`WiFiClient` → `socket()`, `bind()`, `listen()`, `accept()`, `recv()`, `send()`; reconnect logic → your event handlers.
- *Break it:* power-cycle the access point mid-stream, flood the socket, and send a partial frame then stall.

> **note:** the lwIP page says that if you use any lwIP API other than BSD sockets, you must make sure it is thread-safe, and recommends enabling `CONFIG_LWIP_CHECK_THREAD_SAFETY` during testing. Stick to BSD sockets. Wi-Fi also shares ADC2 with your analog inputs (the v6.1 oneshot driver arbitrates, but your Stage 3 labs should keep using ADC1).

**Lab 4f (optional, Freenove Project 23.1 and 23.2 Infrared Remote): decode a timing protocol.**
- *Freenove source:* `Sketch_23.1_Infrared_Remote_Control` and `23.2` (a `Freenove_IR_Lib_for_ESP32` receiver on a GPIO; 23.2 controls an LED and buzzer from remote buttons).
- *Build:* decode the remote's NEC-style frames with RMT receive and write your own decoder from the pulse-width timings; then use the buttons to send commands into your Lab 4c command handler.
- *Swap list:* the IR library → `rmt_new_rx_channel()` and `rmt_receive()` (include `driver/rmt_rx.h`, component `esp_driver_rmt`), with your own symbol decoder; `ir_recv.available()` → a queue filled from the RMT receive-done callback.
- *Break it:* hold a button down (repeat codes), point the remote at a bright window, and show your decoder rejects noise.

OSTEP homework simulator `file-integrity`, for hands-on checksum intuition before you design your frame format.

**Final exam (closed book):**
- Draw a byte-level UART frame with start, length and checksum fields, and explain how the receiver resynchronizes after corruption.
- Explain GAP vs GATT, and what a notify characteristic does that a read does not.
- Why would you choose NimBLE over Bluedroid, and when would you be forced to use Bluedroid?
- Your TCP server and your UART parser share one command handler. What must be true of that handler, and what does the lwIP page say about threads and sockets?

## Stage 5: Production-grade practice

**Readings**
- [Partition Tables](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/partition-tables.html) with OSTEP [Flash-based SSDs](https://pages.cs.wisc.edu/~remzi/OSTEP/file-ssd.pdf) (erase blocks and wear).
- [Fatal Errors](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/fatal-errors.html), [Core Dump](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/core_dump.html), [Watchdogs](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-reference/system/wdts.html), [Unit Testing (Target)](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/unit-tests.html) and [JTAG Debugging](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/jtag-debugging/index.html).
- [IDF Monitor](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/tools/idf-monitor.html) and [`idf.py`](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/api-guides/tools/idf-py.html), which you have been using all course, now read properly.

**Labs and problem sets**
- OSTEP homework simulator `file-ssd`.

**(Prof. Claude) Lab 5: make your Stage 4 device survive failure.** Write a custom partition table with `nvs` and `coredump` partitions, deliberately trigger a panic, capture the core dump and use it to find the faulty line. Then hang a task on purpose, let the task watchdog catch it, and show the device reboots into a known-safe state and records why. If you have the adapter, repeat the diagnosis with JTAG.
- *Swap list:* (Arduino has no equivalent) panic → `CONFIG_ESP_SYSTEM_PANIC_PRINT_REBOOT` (the default, prints registers and a backtrace then restarts) or `CONFIG_ESP_SYSTEM_PANIC_GDBSTUB`; core dump → `CONFIG_ESP_COREDUMP_TO_FLASH_OR_UART` and the `espcoredump` component, read with `idf.py coredump-info` and `idf.py coredump-debug`; hang detection → `esp_task_wdt_init()` with an `esp_task_wdt_config_t` (`timeout_ms`, `idle_core_mask`, `trigger_panic`), `esp_task_wdt_add()` and `esp_task_wdt_reset()`; why you rebooted → `esp_reset_reason()`.
- *Break it:* a null-pointer write, a stack overflow, an infinite loop at higher priority than the idle task, and a divide by zero; show each result in the panic output and the core dump.

> **warning:** core dump to flash needs a partition of type `data` and subtype `coredump` in your partition table, with `CONFIG_ESP_COREDUMP_TO_FLASH_OR_UART` set to the flash destination. The v6.1 page says the partition's minimum size is 20 + (max tasks number × (12 + TCB size + max task stack size)) bytes, controlled by `CONFIG_ESP_COREDUMP_MAX_TASKS_NUM`, and that the core dump partition is declared for you only when you use the default partition table. The page also says the core dump file includes a SHA256 checksum for integrity. `[-Wcoredump-partition]`

> **note:** the v6.1 core dump page documents `idf.py coredump-info` and `idf.py coredump-debug` as wrappers around the `esp-coredump` tool. Use those wrappers rather than running the tool by hand from a tutorial's instructions. The page also says that if flash encryption is enabled, these two commands cannot read an encrypted core dump partition directly.

> **warning:** make sure your custom partition table fits your module's actual flash size (4, 8 or 16 MB depending on your part number, see the pin-rules notes), or it will not boot. The v6.1 page puts the partition table at offset 0x8000 with the first usable offset at 0x9000, and requires app partitions to be aligned to 0x10000 and other partitions to 0x1000. `[-Wpartition-size]`

> **note:** the v6.1 watchdog page says the Task Watchdog is enabled by default (`CONFIG_ESP_TASK_WDT_EN`, `CONFIG_ESP_TASK_WDT_INIT`), with the timeout set by `CONFIG_ESP_TASK_WDT_TIMEOUT_S` and the "trigger panic on timeout" behavior by `CONFIG_ESP_TASK_WDT_PANIC`, and that the interrupt watchdog is configured with `CONFIG_ESP_INT_WDT` and `CONFIG_ESP_INT_WDT_TIMEOUT_MS`. In v6.1 `esp_task_wdt_init()` takes a config struct (and its header says it must be called only after the scheduler has started), so code from older tutorials that passes two integers will not compile. Because the TWDT is initialized at startup by default, the header also offers `esp_task_wdt_reconfigure()` for changing a running watchdog (it must not be called by several tasks at once). The `examples/system/task_watchdog` example's README says its hardware is a chip from the ESP32-S or ESP32-C series, though its target table lists the ESP32; use it for API shape only.

> **error:** JTAG on the Freenove board needs an external adapter, and it uses GPIO12 (TDI), GPIO13 (TCK), GPIO14 (TMS) and GPIO15 (TDO). GPIO12 is the flash-voltage strapping pin, and a high level at reset can prevent booting or flashing, so do not let the adapter or your wiring drive it high at reset. The JTAG pins also collide with any sensor or test point you put on GPIO12-GPIO15, and with the SD slot (GPIO14 and GPIO15). The v6.1 JTAG page documents `openocd -f board/esp32-wrover-kit-3.3v.cfg` for ESP-WROVER-KIT, whose built-in FT2232H adapter your board does not have, so for an external adapter use the page's guidance on other adapters. `[-Wstrapping-pin]`

> **note:** the v6.1 unit testing page says target tests use the Unity framework, live in a component's `test` subdirectory (files starting with `test`), and that the test component must list `unity` in `REQUIRES`. It lists `idf.py menuconfig`, `idf.py build` and `idf.py flash` for the central unit test app, recommends the pytest-embedded framework for CI, and describes multi-device tests that synchronize with `unity_wait_for_signal` and `unity_send_signal`. It also describes Linux-host tests with CMock mocking, which is another way to test your protocol parser off the device. Follow the page for the exact workflow rather than commands from older tutorials.

**Stretch (optional, Freenove Chapters 30 and 31): log to the SD card.** Mount the kit's microSD card with `esp_vfs_fat_sdmmc_mount()` (reference: the v6.1 `storage/sd_card/sdmmc` example, which lists the ESP32) and write your Lab 5 crash reason and a stretch of IMU data to a file, then read it back on a PC. Warnings from the example README: the ESP32 SDMMC pins are fixed (GPIO14 CLK, GPIO15 CMD, GPIO2 D0), the card's lines need pull-ups, GPIO2 pulled low enters download mode, and the example can format the card, so back it up first. Do not run this lab while your JTAG wiring is attached.

**Stretch (optional): the camera.** The camera is the reason this board has PSRAM; the v6.1 tree has `examples/peripherals/camera`. It claims 14 header pins (see the pin rules) and is outside this course's goals.

## Capstone: inertial measurement pod (Prof. Claude)

Build a small inertial measurement device in the spirit of the pods you work with at your job, and treat it like a product. Write the spec first; the acceptance criteria below are a starting point you should tighten. The IMU is the kit's **MPU6050** (6-axis: 3-axis accelerometer and 3-axis gyro, with a temperature sensor and an interrupt pin), driven by your own Lab 3a driver.

**The device** reads the MPU6050 over I2C at a fixed rate, estimates an angle from the angular rate with drift handling (choose a complementary filter, a Kalman filter or Madgwick's filter, and defend the choice with data; with no magnetometer, yaw will drift, and your report should say so), detects an event such as a peak angular velocity and reports it with a timestamp, and streams framed data over your UART protocol, your BLE service and your TCP transport (Lab 4c and 4e). It stores its configuration and calibration in NVS, and recovers from a hung task. Make good use of the kit: show the tilt on the LED bar or LCD, sound the passive buzzer on the event, and consider using the MPU6050's INT pin as a data-ready interrupt (the register map's `INT_PIN_CFG` and `INT_ENABLE` registers) to clock your sampling instead of a timer.

**Acceptance criteria (measure each one):**
- Sample-rate jitter within a limit you set, measured on the logic analyzer if you own one, or by timestamps with the measurement error stated (Stage 1 and 3 tools).
- Zero lost frames over a one-hour run, proven with sequence numbers.
- The Lab 4a fuzz suite passes against the final firmware.
- After a forced task hang, the device recovers into a known-safe state within a stated time and logs the reason.
- A deliberately induced panic produces a core dump you can decode.
- Configuration and calibration survive a power cycle.
- Heap numbers (internal and PSRAM) from your Lab 1 monitor stay flat over the one-hour run.

**Deliverables:** the source repository with unit tests, a two-page report containing the measurements above and a "known limitations" section, a small PC-side script or notebook that plots the stream, and a short demo video. Grade yourself against the 0-3 scale from the syllabus above, item by item.

---

## Sources and verification notes

**Read directly on 2026-09-30:** the v6.1 ESP-IDF doc pages linked above (every URL returned HTTP 200 when I checked), the v6.0 migration guides (peripherals, system, tools), the `v6.1` tag of the ESP-IDF source for headers, Kconfig and example READMEs and directory listings, the WROVER-E datasheet v2.4, the ESP32 Series Datasheet v5.3, the Freenove tutorial PDF, sketches and datasheets from your kit folder, and the UTEP and UT Dallas syllabus PDFs.

**Carried over from the earlier version and not re-read:** OSTEP filenames and homework directories, the Bucknell course page (its server was unreachable today), the Digi-Key series, the Nordic course, Dunlosky et al., and the book and tutorial links. Chapter lists for Adafruit's BLE guide and section-level contents for *Practical Electronics for Inventors* could not be retrieved, so those are pointed at by topic.

**Not verified:** none of the labs was compiled or run on hardware. The Freenove circuit diagrams are images in the tutorial PDF and I could not read them, so wiring notes come from the tutorial text and sketches, including whether any Freenove circuit adds a level shifter. I do not know which WROVER-E variant (flash and PSRAM sizes) your module is. The exact PCNT glitch-filter limit and the exact menuconfig option names for enabling PSRAM were not confirmed; the labs tell you to find them.
