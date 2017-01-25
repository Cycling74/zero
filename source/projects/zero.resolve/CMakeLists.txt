cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


#############################################################
# MAX EXTERNAL
#############################################################


include_directories(
	"${C74_INCLUDES}"
)

if (WIN32)
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../bonjour_sdk")
endif ()


set( SOURCE_FILES
	${PROJECT_NAME}.cpp
)


add_library(
	${PROJECT_NAME}
	MODULE
	${SOURCE_FILES}
	${ZEROCONF_INCLUDES}
)


if (WIN32)
	target_link_libraries(${PROJECT_NAME} "wsock32.lib")
	target_link_libraries(${PROJECT_NAME} "ws2_32.lib")
	target_link_libraries(${PROJECT_NAME} "Iphlpapi.lib")

	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/../bonjour_sdk/x64/dnssd.lib")
	else ()
		target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/../bonjour_sdk/Win32/dnssd.lib")
	endif ()
endif ()


include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)


#############################################################
# UNIT TEST
#############################################################

include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)
