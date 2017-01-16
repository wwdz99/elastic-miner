cmake_minimum_required (VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)
project(ElasticPLFunctions)

message(STATUS "CMAKE - ElasticPLFunctions")

# Confirm SSL Has Been Installed
find_library(SSL_LIBRARY libssl.a REQUIRED)

IF (SSL_LIBRARY)
	message(STATUS "  SSL:    Found")
ELSE (SSL_LIBRARY)
	message(STATUS "  SSL:    Not Found!")
ENDIF (SSL_LIBRARY)

# Confirm GMP Has Been Installed
find_library(GMP_LIBRARY libgmp.a REQUIRED)

IF (GMP_LIBRARY)
	message(STATUS "  GMP:    Found")
ELSE (GMP_LIBRARY)
	message(STATUS "  GMP:    Not Found!")
ENDIF (GMP_LIBRARY)

ADD_LIBRARY( ElasticPLFunctions STATIC
	ElasticPLMath.c
)

target_link_libraries(ElasticPLFunctions gmp)

install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR})
