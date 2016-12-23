cmake_minimum_required (VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)
project(ElasticPLFunctions)

message(STATUS "CMAKE - ElasticPLFunctions")

# Confirm CRYPTO Has Been Installed
find_library(CRYPTO_LIB libcrypto.a REQUIRED)

IF (CRYPTO_LIB)
	message(STATUS "  Crypto: Found")
ELSE (CRYPTO_LIB)
	message(STATUS "  Crypto: Not Found!")
ENDIF (CRYPTO_LIB)

# Confirm SSL Has Been Installed
find_library(SSL_LIBRARY libssl.a REQUIRED)

IF (SSL_LIBRARY)
	message(STATUS "  SSL:    Found")
ELSE (SSL_LIBRARY)
	message(STATUS "  SSL:    Not Found!")
ENDIF (SSL_LIBRARY)

ADD_LIBRARY( ElasticPLFunctions STATIC
	ElasticPLCrypto.c
	ElasticPLMath.c
	../crypto/sha2.c
	../crypto/whirlpool.c
)

target_link_libraries(ElasticPLFunctions crypto)

install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR})
