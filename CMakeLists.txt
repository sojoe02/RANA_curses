cmake_minimum_required(VERSION 2.6)
project(Kasterborous)

#Version number:
set (Kasterborous_VERSION_MAJOR 0)
set (Kasterborous_VERSION_MINOR 794)
#Configure a header file to pass CMake stuff to the source code:
configure_file (
  ${PROJECT_SOURCE_DIR}/src/kasterborous.h.in
  ${PROJECT_BINARY_DIR}/kasterborous.h
)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Curses REQUIRED)
find_package(Lua52 REQUIRED)


ADD_SUBDIRECTORY(src)


#set(Lua52_DIR ${PROJECT_SOURCE_DIR}/)
#message(${Lua52_DIR})


