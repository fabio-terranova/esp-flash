cmake_minimum_required(VERSION 3.10)

project(esp-flash)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    src/Common.cpp
    src/ESP32.cpp
    src/Serial.cpp
    src/main.cpp
)

add_executable(esp-flash ${SOURCES})

target_include_directories(esp-flash PRIVATE include)