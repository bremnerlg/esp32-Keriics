# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/lloydbg/.espressif/v6.1-beta1/esp-idf/components/bootloader/subproject")
  file(MAKE_DIRECTORY "/home/lloydbg/.espressif/v6.1-beta1/esp-idf/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "/home/lloydbg/Documents/Projects/esp32-projects/src/commandline_esp32/commandline_esp32/build/bootloader"
  "/home/lloydbg/Documents/Projects/esp32-projects/src/commandline_esp32/commandline_esp32/build/bootloader-prefix"
  "/home/lloydbg/Documents/Projects/esp32-projects/src/commandline_esp32/commandline_esp32/build/bootloader-prefix/tmp"
  "/home/lloydbg/Documents/Projects/esp32-projects/src/commandline_esp32/commandline_esp32/build/bootloader-prefix/src/bootloader-stamp"
  "/home/lloydbg/Documents/Projects/esp32-projects/src/commandline_esp32/commandline_esp32/build/bootloader-prefix/src"
  "/home/lloydbg/Documents/Projects/esp32-projects/src/commandline_esp32/commandline_esp32/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/lloydbg/Documents/Projects/esp32-projects/src/commandline_esp32/commandline_esp32/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/lloydbg/Documents/Projects/esp32-projects/src/commandline_esp32/commandline_esp32/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
