cmake_minimum_required(VERSION 3.12)
project(CS2Aimbot)

if(NOT WIN32)
    message(FATAL_ERROR "This project is designed to work only on Windows systems")
    return()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCE CONFIGURE_DEPENDS "*.h" "*.cpp")

add_library(CS2Aimbot SHARED ${SOURCE})