# ESP32 Kernix
This is the first iteration of a project I've dubbed as "Kernel-Exposed-Redundantly Network-Inaccesible Computer System." It is meant to serve a similar purpose as something like temple-OS (except not quite as fanatic or controversial).

I am going to be using it for my ESP32 projects so that I can avoid "recompiling the world," every time I want to do a live test on a circuit I've built using the GPIO on my esp32 computer. In essence, I want to be able to control what pin is being used for what in real-time without writing a new program for it...

## Installation Requirements
You need to have the Espressif IDF version 6.1 or greater in order to run this project. It was designed for an ESP32-WROVER-E originally, and as of now will not be tested on other models. Compatibility with other models is not guaranteed. VS Code has an excellent extension for managing esp-idf projects so if you're just starting out, that would probably be a good place to start with developing on esp32. Otherwise, if you are running CLI, the commands you need are:

```
idf.py build
idf.py flash -p [PORT]
idf.py monitor -p [PORT]
```

## Contributing
At the moment, I am keeping this a very tiny, very personal project. I am also limiting the use of AI to only critiquing code that was written by hand (by me) to see if I programmed in any bugs, because this is largely a project I'm using to learn, not some production slop-fest of feature bloat and fancy designs. 
