cmake_minimum_required (VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)
project(elasticpl_crypto)

find_library(CRYPTO_LIB libcrypto.a REQUIRED)

# Confirm SSL Has Been Installed
find_library(SSL_LIBRARY libssl.a REQUIRED)

IF (CRYPTO_LIB)
	message(STATUS "SSL: Found")
ELSE (CRYPTO_LIB)
	message(STATUS "SSL: Not Found!")
ENDIF (CRYPTO_LIB)

ADD_LIBRARY( elasticpl_crypto STATIC
	elasticpl_crypto.c
	sha2.c
)

target_link_libraries(elasticpl_crypto crypto)

install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR})
