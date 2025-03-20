cmake_minimum_required (VERSION 3.20.0)

if(CONFIG_EVENT_BUS)
set(WITH_ZEPHYR 1)
set(WITH_ZEPHYR_LIB 1)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/zephyr.cmake)
endif()