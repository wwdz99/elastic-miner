cmake_minimum_required (VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)
project(xel_miner)

# Confirm CURL Has Been Installed
FIND_PATH( CURL_INCLUDE_DIR curl/curl.h  PATHS ${PC_CURL_INCLUDEDIR} ${PC_CURL_INCLUDE_DIRS}  PATH_SUFFIXES include )
FIND_LIBRARY( CURL_LIBRARIES NAMES curl PATHS ${PC_CURL_LIBDIR} ${PC_CURL_LIBRARY_DIRS} PATH_SUFFIXES lib )

IF (CURL_INCLUDE_DIR AND CURL_LIBRARIES)
	SET(CURL_FOUND TRUE)
ELSE (CURL_INCLUDE_DIR AND CURL_LIBRARIES)
	SET(CURL_FOUND FALSE)
ENDIF (CURL_INCLUDE_DIR AND CURL_LIBRARIES)

IF (CURL_FOUND)
	message(STATUS "CURL: Found")
ELSE (CURL_FOUND)
	message(STATUS "CURL: Not Found!")
ENDIF (CURL_FOUND)


add_subdirectory(compat/jansson)

include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/algo ${PROJECT_SOURCE_DIR}/compat/jansson ${CURL_INCLUDE_DIR})

set(SRC_LIST	xel_miner.c
		util.c
		./ElasticPL/ElasticPL.c
		./ElasticPL/ElasticPLInterpreter.c
		./ElasticPL/ElasticPLParser.c
		./ElasticPL/ElasticPLTokenManager.c
		./crypto/md5.c
		./crypto/sha2.c
)
			
set(TARGET_NAME xel_miner)

IF ( WIN32 )
	add_executable(${TARGET_NAME} ${SRC_LIST} ./compat/winansi.c)
ELSE ( WIN32 )
	add_executable(${TARGET_NAME} ${SRC_LIST})
ENDIF ( WIN32 )

target_link_libraries(${TARGET_NAME} crypto pthread ${CURL_LIBRARIES} jansson)

IF(WIN32)
	target_link_libraries(${TARGET_NAME} libcurl Ws2_32)
ENDIF(WIN32)

IF(UNIX)
	target_link_libraries(${TARGET_NAME} m)
ENDIF(UNIX)

set_target_properties( ${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR} )
