cmake_minimum_required (VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)
project(ElasticPLFunctions)

find_library(CRYPTO_LIB libcrypto.a REQUIRED)

# Confirm SSL Has Been Installed
find_library(SSL_LIBRARY libssl.a REQUIRED)

IF (CRYPTO_LIB)
	message(STATUS "SSL: Found")
ELSE (CRYPTO_LIB)
	message(STATUS "SSL: Not Found!")
ENDIF (CRYPTO_LIB)

ADD_LIBRARY( ElasticPLFunctions STATIC
	ElasticPLCrypto.c
	ElasticPLMath.c
	../crypto/sha2.c
	../crypto/whirlpool.c
)

target_link_libraries(ElasticPLFunctions crypto)

install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR})
