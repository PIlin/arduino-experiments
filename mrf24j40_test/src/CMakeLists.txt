cmake_minimum_required(VERSION 2.8)



set(TARGET_NAME mrf24j40_test)
#set(TARGET_BOARD leonardo)
#set(TARGET_BOARD uno)
#set(TARGET_BOARD atmega328)

#set(${TARGET_NAME}_PORT /dev/tty.usbmodemfa1311)
#set(${TARGET_NAME}_PORT /dev/tty.usbmodemfd131) # uno#set(${TARGET_NAME}_PORT /dev/tty.usbserial-A9007M68) # due

project( ${TARGET_NAME}_proj )

#print_board_list()
#print_board_settings(lilypad328)

file(GLOB_RECURSE SRCS *.cpp *.c)
file(GLOB_RECURSE HDRS *.hpp *.h)

list(APPEND SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../mrflib/mrf24j.cpp)
list(APPEND HDRS ${CMAKE_CURRENT_SOURCE_DIR}/../../mrflib/mrf24j.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../mrflib/)

function (BUILD_FOR_BOARD TNAME)
    parse_generator_arguments(${TNAME} INPUT
                          ""                  # Options
                          "TID;BOARD;PORT"        # One Value Keywords
                          "SRCS;HDRS"  # Multi Value Keywords
                          ${ARGN})

    set(TBNAME ${TNAME}_${INPUT_TID}_${INPUT_BOARD})

    message(STATUS "Have target ${TBNAME}")

	generate_arduino_firmware(${TBNAME}
		BOARD ${INPUT_BOARD}
		SRCS ${INPUT_SRCS}
		HDRS ${INPUT_HDRS}
		PORT ${INPUT_PORT}
	)

	set_target_properties(${TBNAME} PROPERTIES
		COMPILE_DEFINITIONS "ARDUINO_BOARD_${INPUT_BOARD};ARDUINO_NODE_ID=${INPUT_TID}"
	)

endfunction()


# BUILD_FOR_BOARD(${TARGET_NAME}
# 	TID 0
# 	SRCS ${SRCS}
# 	HDRS ${HDRS}
# 	BOARD "uno"
# 	PORT "/dev/tty.usbmodemfd131"
# )

BUILD_FOR_BOARD(${TARGET_NAME}
	TID 0
	SRCS ${SRCS}
	HDRS ${HDRS}
	BOARD "leonardo"
	PORT "/dev/tty.usbmodemfa1311"
)

BUILD_FOR_BOARD(${TARGET_NAME}
	TID 1
	SRCS ${SRCS}
	HDRS ${HDRS}
	BOARD "atmega328"
	PORT "/dev/tty.usbserial-A9007M68"
)