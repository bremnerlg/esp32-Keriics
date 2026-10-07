# ESP32 Keriics
```
|  /
| / ______                  
|/  |    |   ____  .  .   ____   ____
|\  |-----  |      |  |  |      |___
| \ |_____  |      |  |  |____  ____|
```
This is the first iteration of a project I've dubbed as "Kernel-Exposed-Redundantly Internet-Inaccesible Computer System." This is a RTOS for esp-idf functions so you don't have to recompile any time you want to build a circuit. The main feature is the shell which takes esp32 functions and executes them for you (following the normal C syntax, ignoring the esp_err_t return values).


## Installation Requirements
You need to have the Espressif IDF version 6.1 or greater in order to run this project. It was designed for an ESP32-WROVER-E originally, and as of now will not be tested on other models. Compatibility with other models is not guaranteed. VS Code has an excellent extension for managing esp-idf projects so if you're just starting out, that would probably be a good place to start with developing on esp32. Otherwise, if you are running CLI, the commands you need are:

```
idf.py build
idf.py flash -p [PORT]
idf.py monitor -p [PORT]
```

## Contributing
Ideally we want to use idiomatic C++26 noting the [Espressif page on C++ support](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/cplusplus.html). RTOS tasks are wrapped by \<thread\>, meaning we can prefer C++ concurrency features for many things.

Avoid rewriting the data-structures and functions that have been already written, leveraging the C++ standard library. If you are staunchly opposed to writing anything other than pure C for embedded, then this isn't the project for you. The goal here is to write clean, safe, modern code. At the same time it is understood that sometimes it is indeed quicker to get something to work in pure C, so for prototyping that may be allowed..

You may submit PRs for anything you think would be a worthwhile addition to the codebase, except the shell is currently being handrolled by me, so that's pretty much off limits until I get it working. This is just a fun personal project I've been doing to learn esp32 at the end of the day.
