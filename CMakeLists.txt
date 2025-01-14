cmake_minimum_required(VERSION 3.12)
project(DynamicAimbot)

if(NOT WIN32)
    message(FATAL_ERROR "This project is designed to work only on Windows systems")
    return()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCE CONFIGURE_DEPENDS "*.h" "*.cpp")

add_executable(DynamicAimbot ${SOURCE})