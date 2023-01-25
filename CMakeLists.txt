cmake_minimum_required(VERSION 3.16)
project(gsbutils LANGUAGES CXX VERSION 0.5.19)

message(STATUS "${PROJECT_NAME}: ${PROJECT_VERSION}")

# check main project or not
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -pthread -std=c++2a -Wno-unused-result -Wno-unused-variable")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDebug=1 -DDEBUG=1")
    set(DEBUG_STRING "#define DEBUG 1")
endif()

message(STATUS "${PROJECT_NAME} build type: ${CMAKE_BUILD_TYPE}")

if(${MAIN_PROJECT})
  message(STATUS "${PROJECT_NAME} main project")
  configure_file(version.h.in ../version.h)
else()
  configure_file(../version.h.in ../../gsb_utils/version.h)
endif()

set(CMAKE_BINARY_DIR bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)


message(STATUS ${CMAKE_SOURCE_DIR})
message(STATUS ${CMAKE_BINARY_DIR})
message(STATUS ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -L/usr/local/lib ")

list( APPEND SOURCES
  "debug.cpp"
  "gsbutils.h"
)
set(SUFF "")
if(DEBUG)
set(SUFF "d")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${MAIN_PROJECT})

  if(TEST)
    add_executable(test test/example.cpp)
    target_link_libraries(test PUBLIC gsbutils${SUFF})
  else() 
    add_library(${PROJECT_NAME}${SUFF} STATIC ${SOURCES})
    install(TARGETS ${PROJECT_NAME}${SUFF} DESTINATION  /usr/local/lib)
    set_property(TARGET ${PROJECT_NAME}${SUFF} PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)

    install(FILES gsbutils.h debug.h SStrings.h TTimer.h DDate.h
          DESTINATION  /usr/local/include/gsbutils)
  endif()
else()
  add_library(${PROJECT_NAME}${SUFF} STATIC ${SOURCES})
endif()