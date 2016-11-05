cmake_minimum_required (VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)
project(elasticpl_crypto)

FIND_PACKAGE(OpenSSL REQUIRED)

ADD_LIBRARY( elasticpl_crypto STATIC
	elasticpl_crypto.c
	sha2.c
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR})
