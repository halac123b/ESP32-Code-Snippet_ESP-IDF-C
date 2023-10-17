# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/Duy_Ha/esp/esp-idf/components/bootloader/subproject"
  "E:/Duy_Ha/ESP32-Code-Snippet_ESP-IDF-C/hello_world/build/bootloader"
  "E:/Duy_Ha/ESP32-Code-Snippet_ESP-IDF-C/hello_world/build/bootloader-prefix"
  "E:/Duy_Ha/ESP32-Code-Snippet_ESP-IDF-C/hello_world/build/bootloader-prefix/tmp"
  "E:/Duy_Ha/ESP32-Code-Snippet_ESP-IDF-C/hello_world/build/bootloader-prefix/src/bootloader-stamp"
  "E:/Duy_Ha/ESP32-Code-Snippet_ESP-IDF-C/hello_world/build/bootloader-prefix/src"
  "E:/Duy_Ha/ESP32-Code-Snippet_ESP-IDF-C/hello_world/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Duy_Ha/ESP32-Code-Snippet_ESP-IDF-C/hello_world/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/Duy_Ha/ESP32-Code-Snippet_ESP-IDF-C/hello_world/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
