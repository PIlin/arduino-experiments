cmake_minimum_required(VERSION 2.8)



set(TARGET_NAME mrf24j40_test)
#set(TARGET_BOARD leonardo)
#set(TARGET_BOARD uno)
set(TARGET_BOARD atmega328)

#set(${TARGET_NAME}_PORT /dev/tty.usbmodemfa1311)
#set(${TARGET_NAME}_PORT /dev/tty.usbmodemfa121) # uno?
set(${TARGET_NAME}_PORT /dev/tty.usbserial-A9007M68) # due

project( ${TARGET_NAME}_proj )

#print_board_list()
#print_board_settings(lilypad328)

file(GLOB_RECURSE ${TARGET_NAME}_SRCS *.cpp *.c)
file(GLOB_RECURSE ${TARGET_NAME}_HDRS *.hpp *.h)

list(APPEND ${TARGET_NAME}_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../mrflib/mrf24j.cpp)
list(APPEND ${TARGET_NAME}_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/../../mrflib/mrf24j.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mrflib/)

generate_arduino_firmware(${TARGET_NAME} BOARD ${TARGET_BOARD})
