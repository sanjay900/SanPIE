set(CMAKE_BUILD_TYPE Debug)
cmake_minimum_required(VERSION 3.10)
project(sanpie)
find_package(PkgConfig REQUIRED)
set(LIB_SOURCE_FILES
        src/sol.hpp
        src/input/Input.cpp
        src/input/Wiimote.cpp
        src/DeviceException.cpp
        src/output/VJoy.cpp
        src/output/buttons_ref.h
        src/output/VKeyboard.cpp
        src/Loop.cpp
        src/Utils.cpp
        src/DeviceScanner.cpp
        src/input/SerialIn.cpp
        src/input/Controller.cpp
        src/Serial.cpp
        src/output/SerialOut.cpp
        src/output/SerialOut.h src/output/Output.cpp src/output/Output.hpp src/Device.cpp src/Device.hpp)

add_library(libsanpie ${LIB_SOURCE_FILES})
pkg_check_modules(EVDEV libevdev REQUIRED)
include_directories(${EVDEV_INCLUDE_DIRS})
target_compile_features(libsanpie PUBLIC cxx_std_17)
target_include_directories(libsanpie PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(EXE_SOURCE_FILES src/main.cpp)
add_executable(sanpie ${EXE_SOURCE_FILES})

target_compile_features(sanpie PUBLIC cxx_std_17)

TARGET_LINK_LIBRARIES(libsanpie udev pthread ${EVDEV_LIBRARIES} lua)

TARGET_LINK_LIBRARIES(sanpie libsanpie)
