cmake_minimum_required(VERSION 2.8)



set(TARGET_NAME empty_project)
set(TARGET_BOARD leonardo)

set(${TARGET_NAME}_PORT /dev/tty.usbserial-A7005Faa)

project( ${TARGET_NAME}_proj )

#print_board_list()
#print_board_settings(lilypad328)

file(GLOB_RECURSE ${TARGET_NAME}_SRCS *.cpp *.c)
file(GLOB_RECURSE ${TARGET_NAME}_HDRS *.hpp *.h)


generate_arduino_firmware(${TARGET_NAME} BOARD ${TARGET_BOARD} NO_AUTOLIBS SIZE_VERBOSE)
