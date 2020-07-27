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
	target_link_libraries(${PROJECT_NAME} PUBLIC wsock32)
	target_link_libraries(${PROJECT_NAME} PUBLIC ws2_32)
	target_link_libraries(${PROJECT_NAME} PUBLIC Iphlpapi)

	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		find_library(DNSSD_LIBRARY "dnssd" HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../bonjour_sdk/x64")
	else ()
		find_library(DNSSD_LIBRARY "dnssd" HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../bonjour_sdk/Win32")
	endif ()
    target_link_libraries(${PROJECT_NAME} PUBLIC ${DNSSD_LIBRARY})
endif ()


include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)


#############################################################
# UNIT TEST
#############################################################

include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)
