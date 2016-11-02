cmake_minimum_required (VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)
project(xel_miner)

# Confirm CURL Has Been Installed
FIND_PATH( CURL_INCLUDE_DIR curl/curl.h  PATHS ${PC_CURL_INCLUDEDIR} ${PC_CURL_INCLUDE_DIRS}  PATH_SUFFIXES include )
FIND_LIBRARY( CURL_LIBRARIES NAMES curl PATHS ${PC_CURL_LIBDIR} ${PC_CURL_LIBRARY_DIRS} PATH_SUFFIXES lib )
find_package(OpenSSL REQUIRED)

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
				xel_compiler.c
				curve25519-donna-c64.c
				util.c
				./ElasticPL/ElasticPL.c
				./ElasticPL/ElasticPLTokenManager.c
				./ElasticPL/ElasticPLParser.c
				./ElasticPL/ElasticPLInterpreter.c
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

IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
	target_link_libraries(${TARGET_NAME} m dl)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -fPIC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -fPIC")
ELSEIF( WIN32 )
	target_link_libraries(${TARGET_NAME} libcurl Ws2_32)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -msse -msse2 -msse3 -mmmx -m3dnow")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -msse -msse2 -msse3 -mmmx -m3dnow")
ELSEIF ( CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)" )
	target_link_libraries(${TARGET_NAME} m dl)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -msse -msse2 -msse3 -mmmx -m3dnow -fPIC -fext-numeric-literals")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -fPIC -fext-numeric-literals")
ELSEIF ( UNIX )
	target_link_libraries(${TARGET_NAME} m dl)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -fPIC -fext-numeric-literals")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -fPIC -fext-numeric-literals")
ENDIF()

set_target_properties( ${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR} )
