# Professor Claude's Embedded Systems Course: ESP32-WROOM-32 on ESP-IDF v4.4

## Course description

A self-paced, roughly 20-week computer engineering course taught in six stages (0-5), with a capstone project. It is built around three goals:

1. Learn the operating systems fundamentals that matter for real ESP32 firmware.
2. Take sensor input and turn it into something useful.
3. Communicate with the ESP32 over UART, Bluetooth and other practical protocols.

**Prerequisite:** you can already build, flash and monitor an ESP-IDF project, blink an LED and drive or read a GPIO. The course starts after that.

Every ESP-IDF link points at the v4.4 docs, which describe ESP-IDF FreeRTOS as a modified Vanilla FreeRTOS v10.4.3. Each OSTEP entry names the chapter title and filename, all under `https://pages.cs.wisc.edu/~remzi/OSTEP/`. Found-online items are readings, lectures and reference examples. The labs are open-ended specs I wrote, marked **(Prof. Claude)**, because a finished walkthrough is what keeps you in tutorial hell.

## Lab rules (how to stay out of tutorial hell)

1. **Spec first.** Before writing code, write one paragraph stating what you will build and the measurable acceptance criteria (rates, error counts, recovery behavior).
2. **Design from the datasheet and the v4.4 reference, not from a tutorial.** The Espressif examples are reference material, not assignments: read them to look up API usage, then build without them open.
3. **Measure, don't eyeball.** "It seems to work" is not a result. Use the logic analyzer, the multimeter and your own counters.
4. **Break it on purpose.** Every lab includes a fault you inject yourself: corrupted bytes, an unplugged sensor, a starved task, a hung task.
5. **Write a one-page report per lab:** the spec, the design, the measured results and what failed.
6. **Nothing is thrown away.** Each lab's code becomes a component of the capstone (the task monitor, the driver, the protocol parser). Keep them in one repository and tag each lab.

## Compatibility audit and diagnostics

Every lab was checked against the v4.4 API pages, the v4.4 example READMEs and source, and the WROOM-32 pin rules. **None of the labs was compiled or run on hardware**; the audit is documentation-based. Diagnostics use compiler severity:

- `error:` the assignment will not work as written on WROOM-32 with v4.4 without the stated change or extra hardware.
- `warning:` it will compile, but it can waste hours or damage the setup if you skip the note.
- `note:` a fact worth knowing.

**Golden rule:** if a function is not on a v4.4 API reference page, do not use it. Tutorials and blog posts written for ESP-IDF v5.x use different driver APIs (for example a one-shot ADC driver and a `gptimer` timer driver), and those are not described in the v4.4 pages. The v4.4 pages document the legacy names, such as `adc1_get_raw` and the timer-group driver.

### WROOM-32 pin rules (read before wiring anything)

The datasheet PDF would not parse for me, so these come from the v4.4 [GPIO reference](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/gpio.html), the v4.4 [ESP32-DevKitC guide](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/hw-reference/esp32/get-started-devkitc.html) and esptool's [boot mode selection page](https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html). Confirm against the datasheet's pin tables yourself.

> **error:** GPIO6-GPIO11 are wired to the module's SPI flash (the DevKitC guide labels them D0-D3, CMD and CLK). Never use them. `[-Wflash-pins]`

> **warning:** GPIO34-GPIO39 are input-only with no software pull-up or pull-down. A button on one of these needs an external resistor. `[-Winput-only-pins]`

> **warning:** GPIO0, 2, 5, 12 and 15 are strapping pins. GPIO0 held low at reset enters the serial bootloader. GPIO12 (MTDI) driven high at reset selects 1.8 V flash and can prevent flashing on a 3.3 V flash module. Nothing you wire should pull these around reset. `[-Wstrapping-pin]`

> **note:** GPIO16 and GPIO17 are free on WROOM boards but reserved on WROVER (PSRAM). If you keep this code for the WROVER later, avoid them now. `[-Wwrover-portability]`

> **note:** GPIO12-GPIO15 double as the JTAG pins (Stage 5). GPIO1 and GPIO3 are the console UART0 that your USB programming link uses; keep them free.

## C++ coding standard (applies to every lab)

Firmware shops in automotive and aerospace do not write "whatever C++ compiles". They adopt a published subset of the language and enforce it with a static analyzer. This course uses one standard as the rulebook and two as supporting references, so your lab code is written the way a safety-critical team would write it.

**Primary standard**
- [AUTOSAR *Guidelines for the use of the C++14 language in critical and safety-related systems*](https://www.autosar.org/fileadmin/standards/R19-03/AP/AUTOSAR_RS_CPP14Guidelines.pdf) (Release 19-03, free PDF): the automotive C++ rulebook, covering dynamic memory, exceptions, templates, inheritance and virtual functions. Its rules were merged into MISRA C++:2023 (C++17). The MISRA C++:2023 PDF itself is paid and I found no free copy, so this is the rulebook you can annotate. Earlier releases: [R18-03](https://www.autosar.org/fileadmin/standards/R18-03_R1.4.0/AP/AUTOSAR_RS_CPP14Guidelines.pdf), [R17-10](https://www.autosar.org/fileadmin/standards/R17-10_R1.2.0/AP/AUTOSAR_RS_CPP14Guidelines.pdf).

**Supporting references**
- [JSF AV C++ Coding Standards](https://stroustrup.com/JSF-AV-rules.pdf) (Lockheed Martin, free 141-page PDF, March 2006): the aerospace standard, written for the F-35 and the origin of many MISRA-style rules.
- C++ Core Guidelines (Stroustrup and Sutter): no official PDF exists; the source is Markdown in the [isocpp/CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines) repository. Read sections P, I, F, C, R, ES, E and Con.

**Applying them to ESP-IDF v4.4**
- Enable compiler warnings as errors in your component's `CMakeLists.txt` and run a static analyzer over every lab (cppcheck is free; MISRA rule checking needs a commercial tool or cppcheck's paid add-on).
- Keep exceptions and RTTI disabled (ESP-IDF's default). The [C++ Support page](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/cplusplus.html) explains the cost, and the two standards above both prohibit them in embedded code.
- Allocate once at startup and never in a steady-state loop or ISR. Your Lab 1 heap monitor is how you prove it.
- Prefer fixed-width integer types (`<cstdint>`), `constexpr`, `enum class`, `std::array` and `const` by default over macros, raw arrays and C-style casts.

**Checkpoint:** pick any five rules from your analyzer's report on your Lab 1 code, and explain what failure each one prevents.

## Materials list

**Hardware (required)**
- An ESP32-WROOM-32 development board (the model you use at work, not the newer board in the Freenove kit) and a USB data cable.
- Breadboard, jumper wires, resistors, capacitors and a potentiometer (the Freenove kit should cover most of these; check the kit contents yourself).
- A multimeter, needed for the calibration and electronics labs.
- A **6- or 9-axis IMU on an I2C breakout** for Stage 3 and the capstone. An MPU9250-class part is a good choice because the v4.4 `i2c_simple` example is written for an MPU9250, so it doubles as a register-level reference. Any other I2C sensor works for Stage 3 if you adapt the capstone spec to it.
- A smartphone with the free [nRF Connect for Mobile](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-mobile) app (Android and iOS) for the BLE labs.

**Hardware (strongly recommended)**
- A **3.3 V** USB-to-UART adapter, so the ESP32 has a second serial endpoint (the v4.4 `uart_echo` example is documented for a 3.3 V dongle).
- An 8-channel 24 MHz USB logic analyzer with the free PulseView software; SparkFun's [tutorial](https://learn.sparkfun.com/tutorials/using-the-usb-logic-analyzer-with-sigrok-pulseview) shows it decoding UART, I2C and SPI. SparkFun's own listing is marked retired, but any sigrok-compatible 8-channel 24 MHz analyzer follows the same tutorial.
- A second ESP32 board, required for the BLE `throughput_app` benchmark and useful as a BLE or UART peer.
- A level shifter if you will interface any 5 V parts.

**Hardware (optional)**
- A single-supply, rail-to-rail op amp that works from 3.3 V, for the op-amp half of Lab 3c. Starter kits rarely include one, so check yours.
- A JTAG adapter such as ESP-Prog for Stage 5. The [v4.4 JTAG page](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/jtag-debugging/index.html) says ESP-WROVER-KIT has JTAG built in, so a plain WROOM dev board needs an external adapter.
- A CAN transceiver (for example an SN65HVD23x) and a second CAN node, only if you want to try TWAI in Stage 4.

**Software**
- ESP-IDF v4.4 via the [v4.4 Get Started guide](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/get-started/index.html), optionally with VS Code via the [v4.4 VS Code setup page](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/get-started/vscode-setup.html).
- Git (one repository for all your labs), Python for the OSTEP simulators and for PC-side test scripts (the OSTEP README shows `python ./scheduler.py ...` without stating a version), and PulseView.

**Books and references**
- Free: [OSTEP](https://pages.cs.wisc.edu/~remzi/OSTEP/), [Mastering the FreeRTOS Real Time Kernel](https://en.freertos.org/Documentation/02-Kernel/07-Books-and-manual/01-RTOS_book), [The Scientist and Engineer's Guide to DSP](https://www.dspguide.com/pdfbook.htm), [Kalman and Bayesian Filters in Python](https://github.com/rlabbe/Kalman-and-Bayesian-Filters-in-Python) (free, as Jupyter notebooks) and, as an optional companion text, [Lee and Seshia's *Introduction to Embedded Systems*](https://ptolemy.org/books/leeseshia/download.html) (2nd ed., free PDF).
- Paid: [*Practical Electronics for Inventors*, 4th ed.](https://www.adafruit.com/product/1261) (Ch. 2, 6, 8 and 12 only) and Elecia White's [*Making Embedded Systems*, 2nd ed.](https://www.oreilly.com/library/view/making-embedded-systems/9781098151539/).
- Reference: the [WROOM-32 datasheet](https://documentation.espressif.com/esp32-wroom-32_datasheet_en.pdf) and the [Technical Reference Manual](https://documentation.espressif.com/esp32_technical_reference_manual_en.pdf).

**Other**
- A paper lab notebook and a fixed weekly calendar block (see below).

## How this course runs (mimicking a college course)

The structure is borrowed from real courses. Bucknell's [Internet of Things course](https://csci.courses.bucknell.edu/332/spring-2022-the-internet-of-things/) is a good model: it uses ESP-IDF (v4.3) with FreeRTOS and ESP32 peripherals, meets twice a week, grades weekly progress reports on a 0-3 scale (the best 10 count), and ends in an individual project portfolio. UTEP's ECE 4154/5190/6190 [syllabus](https://digitalmeasures.utep.edu/ai/mbarua/schteach/Syllabus-ECE4154_5190_6190-Fall_2025CRN148511502817694-2.pdf) is another ESP-IDF course on WROOM-32 kits; the PDF would not parse for me, so skim it yourself.

- **Weekly rhythm (about 8 hours):** two 80-minute "lecture" blocks (readings and videos), one 2-3 hour lab, and one problem-set session. Bucknell's classes run 80 minutes, which is where the block length comes from; the total is my suggestion.
- **Weekly progress report:** at the end of each week, score yourself 0-3 (0 = did not finish, 3 = lab meets its spec, report written and you can explain it unaided) and write three sentences in your lab notebook.
- **Study method:** the research says to test yourself and space it out. In [Dunlosky et al. (2013)](https://www.psychologicalscience.org/news/releases/which-study-strategies-make-the-grade.html), practice testing and distributed practice rated highest, while rereading, highlighting and summarizing rated low. So close the book and write out the answer before checking, and revisit earlier stages' material every week.
- **Office hours:** write down unanswered questions during the week and bring them to me in one batch.
- **Exams:** a closed-book midterm after Stage 2 and a final after Stage 4 (sample prompts are at the end of those stages). Grade yourself against the readings.
- **Pacing (suggested):** Stage 0 one week, Stage 1 three weeks, Stage 2 three weeks, Stage 3 four weeks, Stage 4 four weeks, Stage 5 two weeks, capstone three weeks.

## Stage 0: Foundations and orientation

**Readings**
- *Practical Electronics for Inventors*, [Ch. 2 Theory](https://www.adafruit.com/product/1261): the circuit fundamentals (Ohm's law, voltage dividers, RC behavior) that every GPIO, pull-up and ADC input depends on. Read only what you can't already do on paper.
- *Practical Electronics for Inventors*, [Ch. 12 Digital Electronics](https://www.adafruit.com/product/1261), for logic levels and digital signaling only. This matters as soon as you wire a 3.3 V ESP32 to 5 V parts.
- [ESP32-WROOM-32 datasheet](https://documentation.espressif.com/esp32-wroom-32_datasheet_en.pdf) for pinout, strapping pins and electrical limits. This revision is marked "Not Recommended for New Designs", but it matches the module used at work.
- Skim the [Technical Reference Manual](https://documentation.espressif.com/esp32_technical_reference_manual_en.pdf) (System and Memory, Interrupt Matrix) and come back to it as a reference, not cover to cover.

**Labs**
- **(Prof. Claude)** Lab 0: build a pin budget for the whole course. List every GPIO you will use across Stages 1-5 (a pulse output and input, ADC input, I2C bus, second UART, any test points), assign each a pin, and check every assignment against the pin rules above and the datasheet's pin tables. Justify each choice in a sentence.
- **(Prof. Claude)** Paper lab: an ESP32 input pin must never see more than 3.3 V, so pick a resistor pair that divides a **5 V** signal down to 3.3 V or less, and calculate the output. Then build it with a 5 V source and check the real output with your multimeter **before** connecting it to any GPIO.

**Checkpoint:** explain, unaided, what a strapping pin is and why you should not tie one to an arbitrary sensor.

## Stage 1: OS foundations, mapped onto the chip

**Readings**
- OSTEP: [Introduction](https://pages.cs.wisc.edu/~remzi/OSTEP/intro.pdf), [Processes](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-intro.pdf), [Direct Execution](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-mechanisms.pdf) (traps, interrupts and context switches are what FreeRTOS does on the ESP32) and [CPU Scheduling](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched.pdf).
- [Mastering the FreeRTOS Real Time Kernel](https://en.freertos.org/Documentation/02-Kernel/07-Books-and-manual/01-RTOS_book) (free), covering tasks, queues and software timers.
- [Application Startup Flow](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/startup.html): boot to scheduler start to `app_main`, which makes sense once you know what a task and a scheduler are.
- [Memory Types](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/memory-types.html) with OSTEP [Address Spaces](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf) and [Free Space Management](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-freespace.pdf). Read these for contrast: the ESP32 has no per-process address spaces, and the heap and DMA-capable buffers are yours to manage.

**Lectures and videos**
- Digi-Key's *Introduction to RTOS* by Shawn Hymel ([playlist listing](https://videohighlight.com/playlist/PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz)). Use it as lecture material; you can skip Part 2 (a getting-started task demo) and treat its challenges as optional. This stage covers [Part 1, what an RTOS is](https://www.digikey.com/es/videos/d/digi-key-electronics/introduction-to-rtos-part-1-what-is-a-real-time-operating-system-rtos), [Part 3, task scheduling](https://www.youtube.com/watch?v=95yUbClyf3E) and [Part 4, memory management](https://www.youtube.com/watch?v=Qske3yZRW5I). (The YouTube links are built from the video IDs in the playlist listing.)

> **warning:** the series is written for the Arduino framework (`setup()`, `loop()`, `Serial`, `analogRead`) on an Adafruit Feather HUZZAH32, which is an ESP32-WROOM-32 board, so the hardware matches. Its FreeRTOS calls are the same ones you will use in ESP-IDF, but the Arduino calls do not exist in ESP-IDF, so replace `Serial` with `printf` or the ESP-IDF logging macros in anything you port. Do not assume Arduino's `loop()` core assignment carries over to `app_main`. `[-Wsdk-version]`

**Labs and problem sets**
- OSTEP homework simulators ([repository](https://github.com/remzi-arpacidusseau/ostep-homework/)): `cpu-intro`, `cpu-sched` and `vm-freespace`. Work the problems by hand first, then check with the simulator's `-c` flag. If `./scheduler.py` will not run directly, the README's alternative is `python ./scheduler.py ...`.
- **(Prof. Claude)** Lab 1: build a reusable **task and heap monitor** that reports, at a configurable interval, each task's state, priority and stack high-water mark, the CPU time split between tasks and between the two cores, and the free and minimum-ever free heap. Then use it to answer three questions with data: (1) how does CPU load divide across the two cores when you pin busy tasks versus leave them unpinned; (2) what stack size does a given task actually need, and how much margin did you leave; (3) how long does it take from a timer alarm to a waiting task running, under idle and heavy load (measure it on the logic analyzer). Acceptance criteria include the monitor's own measured overhead. This module becomes your debugging tool for every later lab.

> **note:** the [v4.4 FreeRTOS page](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/system/freertos.html) documents `xTaskCreatePinnedToCore`, `tskNO_AFFINITY`, `vTaskGetInfo`, `uxTaskGetStackHighWaterMark`, `uxTaskGetSystemState`, `vTaskList`, `vTaskGetRunTimeStats` and `xTaskGetIdleRunTimeCounter`. For the heap, `esp_get_free_heap_size` and `esp_get_minimum_free_heap_size` are documented on the v4.4 [System API page](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/system/system.html), and `heap_caps_get_free_size` on the [heap allocation page](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/system/mem_alloc.html).

> **warning:** the trace-facility and run-time-stats functions only exist if the matching FreeRTOS options are enabled (the page names `configUSE_TRACE_FACILITY`, `configGENERATE_RUN_TIME_STATS` and `configUSE_STATS_FORMATTING_FUNCTIONS`). ESP-IDF's docs say `FreeRTOSConfig.h` is private and you configure FreeRTOS through menuconfig, so search menuconfig for the equivalent options; I did not verify their exact names. `[-Wunverified-api]`

> **warning:** the call that reports the current core (commonly `xPortGetCoreID()`) is not documented on the v4.4 FreeRTOS page I checked. Confirm it exists in your IDF install's FreeRTOS port headers before relying on it. `[-Wunverified-api]`

**Checkpoint:** draw the state diagram of a FreeRTOS task, and say which OSTEP concept each transition corresponds to.

## Stage 2: Concurrency and interrupts (where firmware bugs come from)

**Readings**
- OSTEP: [Concurrency and Threads](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-intro.pdf), [Locks](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf), [Condition Variables](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-cv.pdf), [Semaphores](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-sema.pdf) and [Concurrency Bugs](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-bugs.pdf).
- [FreeRTOS API reference (v4.4)](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/system/freertos.html) for queues, semaphores, event groups and stream buffers, the practical counterparts of the chapters above.
- OSTEP [Multi-CPU Scheduling](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-multi.pdf), then [ESP-IDF FreeRTOS (SMP) in v4.4](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/freertos-smp.html): task pinning across the two Xtensa cores, and spinlock critical sections in place of "disable interrupts".
- [Interrupt Allocation](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/system/intr_alloc.html) (shared interrupts, IRAM-safe handlers).
- OSTEP [Event-based Concurrency](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-events.pdf), then [Event Handling](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/event-handling.html).
- The v4.4 [PCNT](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/pcnt.html) and [LEDC](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/ledc.html) pages, for Lab 2.

**Lectures and videos**
- Digi-Key *Introduction to RTOS*, Parts 5-12: [queues](https://www.youtube.com/watch?v=pHJ3lxOoWeI), [mutexes](https://www.youtube.com/watch?v=I55auRpbiTs), [semaphores](https://www.youtube.com/watch?v=5JcMtbA9QEE), [software timers](https://www.youtube.com/watch?v=b1f1Iex0Tso), [hardware interrupts](https://www.youtube.com/watch?v=qsflCf6ahXU) (with its [solution](https://www.digikey.com/es/maker/projects/introduction-to-rtos-solution-to-part-9-hardware-interrupts/3ae7a68462584e1eb408e1638002e9ed)), [deadlock and starvation](https://www.youtube.com/watch?v=hRsWi4HIENc), [priority inversion](https://www.youtube.com/watch?v=C2xKhxROmhA) and [multicore systems](https://www.digikey.com/en/videos/d/digi-key-electronics/introduction-to-rtos-part-12-multicore-systems). Watch them as lectures; the labs below replace the per-video challenges.

> **error:** Part 9's solution uses Arduino-only calls (`timerBegin`, `timerAttachInterrupt`, `timerAlarmWrite`, `timerAlarmEnable`, `analogRead` and `Serial`). They do not exist in ESP-IDF. If you port it, rebuild the hardware timer with the timer-group driver (see the [timer_group example](https://github.com/espressif/esp-idf/tree/v4.4/examples/peripherals/timer_group) for reference), and do not copy the code. `[-Wsdk-version]`

> **warning:** Part 9's challenge calls the ADC from inside the timer ISR. I found nothing in the v4.4 ADC page saying `adc1_get_raw` is safe to call from an ISR. Have the ISR give a semaphore or task notification and let a task read the ADC. `[-Wisr-safety]`

**Labs and problem sets**
- OSTEP homework simulators: `threads-intro`, `threads-locks`, `threads-cv`, `threads-sema`, `threads-bugs` and `cpu-sched-multi`.
- **(Prof. Claude)** Lab 2: build a **pulse-train measurement instrument** and find where it breaks. Generate a pulse train with LEDC on one pin, jumper it to an input pin, and measure its frequency and jitter two ways: (A) a GPIO interrupt that timestamps each edge into a queue or ring buffer, with a task computing frequency and jitter, and (B) PCNT counting over fixed gate windows. Sweep the frequency upward. Using sequence numbers or timestamp accounting, prove exactly where method A starts losing events, compare with B, and write down why (ISR latency, queue depth, what else the CPU is doing). Then run a flash operation, such as an NVS write, during the measurement and see whether your ISR path stalls; the Interrupt Allocation page covers IRAM-safe handlers. Acceptance criteria: a stated event-loss count at each frequency, and a defended frequency limit for each method.
- **(Prof. Claude)** Lab 2b: reproduce a priority inversion on purpose, prove it with your Lab 1 monitor, fix it, and show the fix in the numbers.

> **warning:** PCNT counters are 16-bit signed (-32768 to 32767), so at high frequencies they overflow fast. Accumulate across gate windows or use its threshold events. The glitch filter is set in APB clock cycles (10-bit, so about 12.8 µs at most on the 80 MHz APB clock); a filter that long will swallow fast pulses. `[-Wcounter-overflow]`

> **warning:** the v4.4 [SMP page](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/freertos-smp.html) says each core independently schedules the highest-priority ready task allowed by its affinity. Tasks on different cores run truly in parallel, so a classic priority-inversion demo can silently fail to reproduce. Pin all three demo tasks to the **same** core. `[-Wpriority-inversion-smp]`

> **note:** LEDC can drive PWM on any output GPIO. On the ESP32 it has 16 channels (8 high-speed, 8 low-speed), and frequency and duty resolution trade off: 5 kHz supports 13-bit duty resolution, and the maximum frequency is 40 MHz at 1-bit resolution.

> **warning:** an input on GPIO34-GPIO39 has no software pull-up. In v4.4 the pin ISR handler needs no `IRAM_ATTR` unless you pass `ESP_INTR_FLAG_IRAM` to `gpio_install_isr_service()`, which is the flag that makes it survive flash operations. `[-Winput-only-pins]`

**Midterm (closed book):**
- Trace what happens from a GPIO edge to a task running, naming every OS concept involved.
- Why is "disable interrupts" not a valid critical section on the ESP32?
- Given a two-task deadlock in code you have not seen, find and fix it.
- When would you choose PCNT over a GPIO interrupt, and what limits each?

## Stage 3: Sensor input to useful data

**Readings**
- OSTEP [I/O Devices](https://pages.cs.wisc.edu/~remzi/OSTEP/file-devices.pdf) (polling vs interrupts vs DMA).
- *Practical Electronics for Inventors*, [Ch. 6 Sensors](https://www.adafruit.com/product/1261) (how common sensor types work and what they output) and [Ch. 8 Operational Amplifiers](https://www.adafruit.com/product/1261) (conditioning a sensor signal to fit the ADC's limited input range, which tops out near 2450 mV at 11 dB attenuation on the ESP32).
- Peripheral drivers: [GPIO](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/gpio.html), [ADC](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html) (ADC2 conflicts with Wi-Fi, so use ADC1 pins, and calibrate), [general purpose timers](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/timer.html), [I2C](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/i2c.html) and [SPI master](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/spi_master.html).
- [Non-Volatile Storage (NVS)](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/storage/nvs_flash.html), for persisting calibration data.
- Free signal-processing text for making sensor data useful: [The Scientist and Engineer's Guide to DSP](https://www.dspguide.com/pdfbook.htm), specifically [Ch. 3 ADC and DAC](http://www.dspguide.com/ch3.htm), [Ch. 15 Moving Average Filters](http://www.dspguide.com/ch15.htm) and [Ch. 19 Recursive Filters](http://www.dspguide.com/ch19.htm).
- Elecia White, [*Making Embedded Systems*, 2nd ed.](https://www.oreilly.com/library/view/making-embedded-systems/9781098151539/) (paid). The second edition adds sensor and data-handling chapters, and it is the best single companion for design patterns.
- For the capstone's angle estimation: [Kalman and Bayesian Filters in Python](https://github.com/rlabbe/Kalman-and-Bayesian-Filters-in-Python) (free), and the [Madgwick orientation filter](https://ahrs.readthedocs.io/en/latest/filters/madgwick.html) explanation, which cites Madgwick's 2010 report on IMU and MARG orientation filters.

**Lectures and tutorials**
- SparkFun's [I2C tutorial](https://learn.sparkfun.com/tutorials/i2c) (open-drain signaling, pull-ups, addressing, clock stretching) and [Serial Communication](https://learn.sparkfun.com/tutorials/serial-communication/all) (synchronous vs asynchronous, which also sets up Stage 4). I could only confirm text tutorials for this stage, not videos.

**Tools and reference examples (not assignments)**
- [i2c_tools](https://github.com/espressif/esp-idf/tree/v4.4/examples/peripherals/i2c/i2c_tools): a console with `i2cdetect`, `i2cget`, `i2cset` and `i2cdump`. Use it to explore your sensor's registers before you write code. On the plain ESP32 its default pins are GPIO18 (SDA) and GPIO19 (SCL), and the driver enables the internal pull-ups.
- [i2c_simple](https://github.com/espressif/esp-idf/tree/v4.4/examples/peripherals/i2c/i2c_simple) as a register-level reference for an MPU9250; the [adc](https://github.com/espressif/esp-idf/tree/v4.4/examples/peripherals/adc) and [timer_group](https://github.com/espressif/esp-idf/tree/v4.4/examples/peripherals/timer_group) examples for API usage.

**Labs**
- **(Prof. Claude)** Lab 3a: write a **datasheet-driven I2C driver** for your sensor, with no vendor library and no copied example. Cover the WHO_AM_I check, initialization, configurable range and sample rate, burst reads, unit conversion, timeouts and bus recovery (a stuck SDA or SCL line). Verify every transaction against the logic analyzer's I2C decode. Unit-test the register and scale-conversion logic. Acceptance criteria: the driver survives unplugging the sensor mid-read and recovers when it is plugged back in, without a reboot.
- **(Prof. Claude)** Lab 3b: produce an **ADC characterization and calibration report**. Sample a divider or potentiometer on ADC1 against multimeter-measured voltages at 10 or more points. Compare raw counts, `esp_adc_cal` millivolts and your own least-squares fit, and quantify noise as a function of oversampling and filtering (moving average, Ch. 15, and recursive, Ch. 19) and of sample rate. Store your calibration coefficients in NVS and show they survive a reset. The deliverable is a one-page report with an error-versus-input plot and the effective resolution you achieved.
- **(Prof. Claude)** Lab 3c (electronics): condition the signal in hardware. Build an RC low-pass anti-alias filter (DSP Ch. 3) and, if you have a rail-to-rail op amp, a buffer or gain stage (Ch. 8). Measure noise and bandwidth with and without them using your Lab 3b setup, and state which improved and by how much.

> **error:** `i2c_simple` is written for an MPU9250 and confirms it by reading the WHO_AM_I register. With any other sensor its code will not work as written. That is why it is a reference here and Lab 3a has you write your own. `[-Wmissing-hardware]`

> **error:** there is no standalone SPI lab. The two v4.4 `spi_master` examples are `hd_eeprom` (needs an external EEPROM) and `lcd` (needs an LCD). Without that hardware, treat the SPI page as reading, or write your own driver test for an SPI sensor you own. `[-Wmissing-hardware]`

> **warning:** ADC pins. The v4.4 ADC page says the Hall sensor uses ADC1 channels on GPIO36 and GPIO39 and not to connect anything else to those pins, so put your analog input on GPIO32-GPIO35. The plain ESP32 documents single-read mode; do not build the lab around DMA or continuous ADC. Use the v4.4 names (`adc1_config_width`, `adc1_get_raw`, `esp_adc_cal_characterize`, `esp_adc_cal_raw_to_voltage`), and calibrate before trusting millivolt values. `[-Wadc-pins]`

> **warning:** NVS needs a partition named `nvs` in your partition table. It is log-structured with wear leveling, but every commit is still a flash write, so persist calibration rarely, never at the sampling rate. `[-Wflash-wear]`

**Checkpoint:** why can you not just call the ADC as often as you like, and what is the minimum sample rate implied by the sampling theorem for the signal you chose?

## Stage 4: Communication (UART, Bluetooth, others)

**Readings**
- [UART driver (v4.4)](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/uart.html): driver installation, interrupts and pattern detection.
- OSTEP [Data Integrity and Protection](https://pages.cs.wisc.edu/~remzi/OSTEP/file-integrity.pdf), the theory behind checksums and framing for your own serial protocols.
- Bluetooth: [Adafruit's Introduction to BLE](https://learn.adafruit.com/introduction-to-bluetooth-low-energy/introduction) for GAP/GATT concepts, then the [v4.4 Bluetooth API overview](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/bluetooth/index.html) and the [v4.4 bluetooth examples](https://github.com/espressif/esp-idf/tree/v4.4/examples/bluetooth). Bluedroid is the default and covers Classic plus BLE. NimBLE is BLE-only and lighter, so choose by whether Classic (SPP/A2DP) is needed.
- Other protocols: [TWAI (CAN)](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/twai.html) and [lwIP TCP/IP](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/lwip.html).

> **error:** TWAI has no lab because it cannot run on a bare board. The TWAI controller has no integrated transceiver, so an external one (the page suggests SN65HVD23x) and a second CAN node are required, and it supports only classic CAN frames, not CAN FD. Treat it as reading unless you own that hardware. `[-Wmissing-hardware]`

**Lectures and tutorials**
- Nordic's free [Bluetooth LE Fundamentals](https://academy.nordicsemi.com/courses/bluetooth-low-energy-fundamentals/) course (about 8-10 hours, six lessons covering GAP roles, GATT, advertising, connections, data exchange, security and sniffing). Read Lessons 1-4 for concepts.

> **error:** the Nordic course's exercises require a Nordic nRF development kit. Skip the exercises; on an ESP32 you cannot complete them. `[-Wmissing-hardware]`

> **error:** Espressif's [BLE Introduction](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32/api-guides/ble/get-started/ble-introduction.html) tells you to flash `examples/bluetooth/ble_get_started/nimble/NimBLE_GATT_Server`. The v4.4 bluetooth examples folder contains only `bluedroid`, `blufi`, `esp_ble_mesh`, `esp_hid_device`, `esp_hid_host`, `hci` and `nimble`, so that project does not exist in v4.4. Use the page for the general phone workflow and the v4.4 NimBLE examples for code reference. `[-Wsdk-version]`

**Reference examples (not assignments)**
- UART: the v4.4 [uart examples](https://github.com/espressif/esp-idf/tree/v4.4/examples/peripherals/uart) (`uart_echo`, `uart_events`, `uart_async_rxtxtasks`) show the driver API; skim them for usage and then build your own.
- BLE (NimBLE, in [examples/bluetooth/nimble](https://github.com/espressif/esp-idf/tree/v4.4/examples/bluetooth/nimble)): `blehr` (heart-rate peripheral that sends notifications) and `bleprph` (peripheral with a GATT database and security). Both list ESP32 as their supported target, and either can be tested with any BLE scanner app such as nRF Connect for Mobile.

**Labs**
- **(Prof. Claude)** Lab 4a: design a **framed command and response protocol** over UART. It needs a start marker, a length field, a command ID, a payload and an error-detecting checksum or CRC (use OSTEP's data-integrity chapter to choose), plus sequence numbers and ack/nack. Commands: get and set configuration, start and stop streaming, read status. Implement the parser as a state machine that you can unit-test off the device. Then write a PC-side script that fuzzes it with bit flips, dropped bytes, truncated frames and back-to-back frames. Acceptance criteria: the parser never crashes or hangs, resynchronizes within one frame after corruption, and you have measured throughput at the highest baud rate your adapter and wiring hold reliably.
- **(Prof. Claude)** Lab 4b: build a **custom BLE GATT service** for the same data: a write characteristic for commands, a notify characteristic for data frames and a read characteristic for status. Handle subscribe and unsubscribe, disconnect and reconnect (restart advertising), and the notification size limit that the negotiated MTU sets. Verify everything in nRF Connect for Mobile. If you own a second board, benchmark against the [throughput_app](https://github.com/espressif/esp-idf/tree/v4.4/examples/bluetooth/nimble/throughput_app) example, whose README quotes roughly 340 kbps for notify, 200 kbps for read and 500 kbps for write, and explain any gap between its numbers and yours.
- **(Prof. Claude)** Lab 4c: make the protocol layer **transport-agnostic**. The same command handler and frame builder should serve both UART bytes and BLE writes, with the transport as a thin adapter underneath. This is the shape of the packet protocols that device firmware and host software such as MyPod's EZAnything library exchange.
- OSTEP homework simulator `file-integrity`, for hands-on checksum intuition before you design your frame format.

> **warning:** connect the adapter to whichever UART1 pins you assigned in Lab 0 (the v4.4 `uart_echo` example defaults to GPIO4 and GPIO5), with TX to RX, RX to TX and a common ground, never to GPIO1 and GPIO3. Those are the console UART0 your USB link uses; the `uart_events` example also uses UART0. Use a 3.3 V adapter. `[-Wconsole-uart]`

> **warning:** the `blehr` and `bleprph` Python test utilities (`blehr_test.py`, `bleprph_test.py`) are Linux-only (they need BlueZ and DBus). On Windows, use nRF Connect for Mobile instead. `[-Wlinux-only]`

> **warning:** the `bleprph` README says bonding is not yet persistent across reboots because of incomplete NVS integration, so do not design your acceptance criteria around a bonded reconnect based on that example. `[-Wunverified-hardware-pairing]`

> **warning:** `blecent` is a GATT client that only connects to a server advertising the Alert Notification service. Its README names its Python test server and any server with that service; it does not name `bleprph` or `blehr`, so do not assume they pair. Skip it unless you can create such a server (nRF Connect's GATT server feature is described for Android only). `[-Wunverified-hardware-pairing]`

> **note:** I could not locate a README for `ble_spp`, so its test procedure is unverified and I left it out.

**Final exam (closed book):**
- Draw a byte-level UART frame with start, length and checksum fields, and explain how the receiver resynchronizes after corruption.
- Explain GAP vs GATT, and what a notify characteristic does that a read does not.
- Why would you choose NimBLE over Bluedroid, and when would you be forced to use Bluedroid?

## Stage 5: Production-grade practice

**Readings**
- [Partition Tables](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/partition-tables.html) with OSTEP [Flash-based SSDs](https://pages.cs.wisc.edu/~remzi/OSTEP/file-ssd.pdf) (erase blocks and wear).
- [Fatal Errors](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/fatal-errors.html), [Core Dump](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/core_dump.html), [Watchdogs](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/system/wdts.html), [Unit Testing (Target)](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/unit-tests.html) and [JTAG Debugging](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-guides/jtag-debugging/index.html).

**Labs and problem sets**
- OSTEP homework simulator `file-ssd`.
- **(Prof. Claude)** Lab 5: make your Stage 4 device **survive failure**. Write a custom partition table with `nvs` and `coredump` partitions, deliberately trigger a panic, capture the core dump and use it to find the faulty line. Then hang a task on purpose, let the task watchdog (`esp_task_wdt_add` and `esp_task_wdt_reset`) catch it, and show the device reboots into a known-safe state and records why. If you have the adapter, repeat the diagnosis with JTAG.

> **warning:** core dump to flash needs a `data` partition with subtype `coredump` of at least 64 KB in your partition table, and the "core dump to flash" option enabled in menuconfig. The v4.4 page does not give the exact option name, so search menuconfig rather than trusting a name from a newer tutorial. `[-Wunverified-api]`

> **warning:** the v4.4 core dump page documents `espcoredump.py` (its `info_corefile` and `dbg_corefile` commands) and does not mention `idf.py coredump-info` or `coredump-debug`. Use the documented script. `[-Wsdk-version]`

> **warning:** make sure your custom partition table fits your module's actual flash size, or it will not boot. `[-Wpartition-size]`

> **note:** the v4.4 watchdog page says both the interrupt watchdog and the task watchdog are enabled by default, with idle tasks automatically subscribed to the task watchdog. The timeout is set with `CONFIG_ESP_TASK_WDT_TIMEOUT_S`.

> **error:** JTAG on a plain WROOM-32 needs an external adapter, and it uses GPIO12 (TDI), GPIO13 (TCK), GPIO14 (TMS) and GPIO15 (TDO). GPIO12 is the flash-voltage strapping pin, and a high level at reset can prevent booting or flashing on a 3.3 V flash module, so do not let the adapter or your wiring drive it high at reset. The JTAG pins also collide with any sensor or test point you put on GPIO12-GPIO15. `[-Wstrapping-pin]`

> **note:** target unit tests use the Unity framework in `tools/unit-test-app`, built with `idf.py -T all build`, and your own component's tests go in its `test` subdirectory. Multi-device tests need manual steps.

## Capstone: inertial measurement pod (Prof. Claude)

Build a small inertial measurement device in the spirit of the pods you work with at your job, and treat it like a product. Write the spec first; the acceptance criteria below are a starting point you should tighten.

**The device** reads an IMU over I2C at a fixed rate, estimates an angle from the angular rate with drift handling (choose a complementary filter, a Kalman filter or Madgwick's filter, and defend the choice with data), detects an event such as a peak angular velocity and reports it with a timestamp, and streams framed data over both your UART protocol and your BLE service. It stores its configuration and calibration in NVS, and recovers from a hung task.

**Acceptance criteria (measure each one):**
- Sample-rate jitter within a limit you set, measured on the logic analyzer (Stage 1 and 3 tools).
- Zero lost frames over a one-hour run, proven with sequence numbers.
- The Lab 4a fuzz suite passes against the final firmware.
- After a forced task hang, the device recovers into a known-safe state within a stated time and logs the reason.
- A deliberately induced panic produces a core dump you can decode.
- Configuration and calibration survive a power cycle.

**Deliverables:** the source repository with unit tests, a two-page report containing the measurements above and a "known limitations" section, a small PC-side script or notebook that plots the stream, and a short demo video. Grade yourself against the 0-3 scale from the syllabus above, item by item.

If you do not have an IMU, substitute another sensor and rewrite the spec to fit it, keeping the same structure of rate, drift or noise handling, event detection, dual transport, persistence and recovery.

---

Sources: the pages linked above. OSTEP filenames and homework directories, the v4.4 API guide and peripheral lists, the v4.4 example READMEs and source, the v4.4 pages for PCNT, LEDC, NVS, watchdogs and the FreeRTOS run-time statistics functions, the Bucknell course page, the Digi-Key series, the Nordic course and the throughput app README were read from the live pages on 2026-09-29. Chapter lists for Adafruit's BLE guide and section-level contents for *Practical Electronics for Inventors* could not be retrieved, so those are pointed at by topic. The WROOM-32 datasheet PDF could not be parsed, so pin rules come from the pages named in the pin-rules section.
