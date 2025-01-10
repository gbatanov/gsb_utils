cmake_minimum_required(VERSION 3.27)
project(gsbutils LANGUAGES CXX VERSION "0.16.4")

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

message(STATUS "${PROJECT_NAME}: ${PROJECT_VERSION}")

set(MAIN_PROJECT ON)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -DWin32=1")
    set(TEST "test")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -pthread  -Wno-unused-result -Wno-unused-variable")
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDebug=1 -DDEBUG=1")
    set(DEBUG_STRING "#define DEBUG 1")
endif()

message(STATUS "${PROJECT_NAME} build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    configure_file(version.h.in ../../../version.h)
else()
    configure_file(version.h.in ../version.h)
endif()

list( APPEND SOURCES
  "gsbutils.h"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VERSION ${PROJECT_VERSION})
set(PROJECT_DESCRIPTION "Библиотека утилит")

if(TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I./../../ -I./../../../ -I./../")
    add_executable(test "test/example.cpp" )
    add_executable(test-chan "test/test-chan.cpp"  )
    add_executable(test-context "test/test-context.cpp"  )
    add_executable(test-pool "test/test-pool.cpp"  )
endif()
