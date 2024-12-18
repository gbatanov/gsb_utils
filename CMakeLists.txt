cmake_minimum_required(VERSION 3.20)
project(gsbutils LANGUAGES CXX VERSION 0.13.54)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

message(STATUS "${PROJECT_NAME}: ${PROJECT_VERSION}")

# check main project or not
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

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
if(${MAIN_PROJECT})
    configure_file(version.h.in ../version.h)
else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)
endif()
endif()

list( APPEND SOURCES
  "debug.cpp"
  "gsbutils.h"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${MAIN_PROJECT})
    set(PREFIX /usr/local)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        set(PREFIX /opt/local)
    endif()
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(PREFIX ./)
    endif()

    set(VERSION ${PROJECT_VERSION})
    set(PROJECT_DESCRIPTION "Библиотека утилит")

    set(INCLUDE_INSTALL_DIR ${PREFIX}/include )
    set(LIB_INSTALL_DIR ${PREFIX}/lib  )
    set(SYSCONFIG_INSTALL_DIR "" )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${INCLUDE_INSTALL_DIR} -I${PROJECT_SOURCE_DIR}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -L${LIB_INSTALL_DIR}")

    if(TEST)
        add_executable(test "test/example.cpp" )
        add_executable(test-chan "test/test-chan.cpp" )
        add_executable(test-context "test/test-context.cpp" )
        target_link_libraries(test PUBLIC gsbutils)
        add_library(${PROJECT_NAME} STATIC ${SOURCES} "version.h")
    else()
        add_library(${PROJECT_NAME} SHARED ${SOURCES} "version.h")
    endif() 

    install(TARGETS ${PROJECT_NAME} DESTINATION  ${LIB_INSTALL_DIR})
    set_property(TARGET ${PROJECT_NAME} PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)

    install(FILES gsbutils.h debug.h SStrings.h TTimer.h DDate.h Channel.h version.h thread_pool.h
          DESTINATION  ${INCLUDE_INSTALL_DIR}/gsbutils)
          
    include(CMakePackageConfigHelpers)
    configure_package_config_file(${PROJECT_NAME}Config.cmake.in
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}
              PATH_VARS INCLUDE_INSTALL_DIR 
              SYSCONFIG_INSTALL_DIR
              )
    write_basic_package_version_file(
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              VERSION ${VERSION}
              COMPATIBILITY SameMajorVersion )  
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}) 
        
                  
    configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
    
else()
      add_library(${PROJECT_NAME}${SUFF} STATIC ${SOURCES})
endif()

