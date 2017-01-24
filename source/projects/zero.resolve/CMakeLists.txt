cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


#############################################################
# MAX EXTERNAL
#############################################################


include_directories(
	"${C74_INCLUDES}"
	"${CMAKE_CURRENT_SOURCE_DIR}/../zeroconf"
)

#set(ZEROCONF_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../zeroconf")
file(GLOB_RECURSE ZEROCONF_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../zeroconf/*.h")

set( SOURCE_FILES
	${PROJECT_NAME}.cpp
)


add_library(
	${PROJECT_NAME}
	MODULE
	${SOURCE_FILES}
	${ZEROCONF_INCLUDES}
)


include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)


#############################################################
# UNIT TEST
#############################################################

include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)