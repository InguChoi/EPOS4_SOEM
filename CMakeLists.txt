cmake_minimum_required(VERSION 3.9)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")
project(SOEM
    DESCRIPTION "Simple Open EtherCAT Master"
    VERSION 1.4.0
    LANGUAGES C)

set(SOEM_INCLUDE_INSTALL_DIR include/soem)
set(SOEM_LIB_INSTALL_DIR lib)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(BUILD_TESTS TRUE)
else()
  message(STATUS "SOEM: not building tests when built as dependency")
  set(BUILD_TESTS FALSE)
endif()


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -lm")
set(OS_LIBS pthread rt)

file(GLOB SOEM_SOURCES soem/*.c)
file(GLOB OSAL_SOURCES osal/*.c)
file(GLOB OSHW_SOURCES oshw/*.c)

file(GLOB SOEM_HEADERS soem/*.h)
file(GLOB OSAL_HEADERS osal/*.h)
file(GLOB OSHW_HEADERS oshw/*.h)

add_library(soem STATIC
  ${SOEM_SOURCES}
  ${OSAL_SOURCES}
  ${OSHW_SOURCES}
  ${OSHW_EXTRA_SOURCES})
target_link_libraries(soem ${OS_LIBS})

target_include_directories(soem PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/soem>
  $<INSTALL_INTERFACE:include/soem>)

target_include_directories(soem PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/osal>
  $<INSTALL_INTERFACE:include/soem>)

target_include_directories(soem PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/osal>
  $<INSTALL_INTERFACE:include/soem>)

target_include_directories(soem
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/oshw>
  $<INSTALL_INTERFACE:include/soem>
  )

message(STATUS "LIB_DIR: ${SOEM_LIB_INSTALL_DIR}")

install(TARGETS soem EXPORT soemConfig DESTINATION ${SOEM_LIB_INSTALL_DIR})

install(EXPORT soemConfig DESTINATION share/soem/cmake)

install(FILES
  ${SOEM_HEADERS}
  ${OSAL_HEADERS}
  ${OSHW_HEADERS}
  DESTINATION ${SOEM_INCLUDE_INSTALL_DIR})

if(BUILD_TESTS) 
  add_subdirectory(test/slaveinfo)
  add_subdirectory(test/epos4)
endif()
