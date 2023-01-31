cmake_minimum_required(VERSION 3.16)
project(gsbutils LANGUAGES CXX VERSION 0.6.16)

message(STATUS "${PROJECT_NAME}: ${PROJECT_VERSION}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I. -fPIC -pthread -std=c++2a -Wno-unused-result -Wno-unused-variable")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDebug=1 -DDEBUG=1")
    set(DEBUG_STRING "#define DEBUG 1")
endif()

message(STATUS "${PROJECT_NAME} build type: ${CMAKE_BUILD_TYPE}")

configure_file(version.h.in ../version.h)

set(CMAKE_BINARY_DIR bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -L/usr/local/lib ")

list( APPEND SOURCES
  "debug.cpp"
  "gsbutils.h"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PREFIX /usr/local)
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
   set(PREFIX /opt/local)
endif()

set(VERSION ${PROJECT_VERSION})
set(PROJECT_DESCRIPTION "Библиотека утилит")

set(INCLUDE_INSTALL_DIR ${PREFIX}/include/ )
set(LIB_INSTALL_DIR ${PREFIX}/lib/  )
set(SYSCONFIG_INSTALL_DIR "" )

if(TEST)
    add_executable(test test/example.cpp)
    target_link_libraries(test PUBLIC gsbutils)
endif() 

add_library(${PROJECT_NAME} SHARED ${SOURCES})
install(TARGETS ${PROJECT_NAME} DESTINATION  ${LIB_INSTALL_DIR})
set_property(TARGET ${PROJECT_NAME} PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)

install(FILES gsbutils.h debug.h SStrings.h TTimer.h DDate.h
          DESTINATION  /usr/local/include/gsbutils)
          
include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              INSTALL_DESTINATION ${LIB_INSTALL_DIR}cmake/${PROJECT_NAME}
              PATH_VARS INCLUDE_INSTALL_DIR 
              SYSCONFIG_INSTALL_DIR
              )
write_basic_package_version_file(
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              VERSION ${VERSION}
              COMPATIBILITY SameMajorVersion )  
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${LIB_INSTALL_DIR}cmake/${PROJECT_NAME}) 
        
                  
    configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${PREFIX}/lib/pkgconfig)
