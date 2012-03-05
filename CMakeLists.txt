cmake_minimum_required(VERSION 2.8)
include(ExternalProject)

project(ows)

add_definitions(-Wall -g -O2)
set(CMAKE_CXX_FLAGS_RELEASE)

#
# That's ugly but has to be devided into several variables
#
FIND_PROGRAM(THRIFT_BIN thrift /bin /usr/bin /usr/local/bin /opt/local/bin)
FIND_PROGRAM(PATCH_BIN patch /bin /usr/bin /usr/local/bin /opt/local/bin)
FIND_PROGRAM(FAKE_INSTALL_BIN echo /bin /usr/bin /usr/local/bin /opt/local/bin)

SET(THRIFT_FILE "${CMAKE_CURRENT_LIST_DIR}/src/model.thrift" )
SET(THRIFT_OUTDIR "${CMAKE_CURRENT_LIST_DIR}/src")
SET(THRIFT_FLAGS_RECURSIVE "-r")
SET(THRIFT_FLAGS_GEN_OPT "--gen")
SET(THRIFT_FLAGS_LANGUAGE "cpp")
SET(THRIFT_FLAGS_OUTPUT "-o")
SET(THRIFT_FLAGS_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/src")

SET(THRIFT_GEN_CPP "${THRIFT_FLAGS_OUTPUT_DIRECTORY}/gen-cpp")

#
# Thrift stuff
#
IF(THRIFT_BIN)
	MESSAGE(STATUS "Thrift found")
	IF(EXISTS ${THRIFT_FILE})
		MESSAGE(STATUS "Generating thrift code: ${THRIFT_BIN} ${THRIFT_FLAGS_RECURSIVE} ${THRIFT_FLAGS_GEN_OPT} ${THRIFT_FLAGS_LANGUAGE} ${THRIFT_FLAGS_OUTPUT} ${THRIFT_FLAGS_OUTPUT_DIRECTORY} ${THRIFT_FILE}")
		execute_process(COMMAND ${THRIFT_BIN} ${THRIFT_FLAGS_RECURSIVE} ${THRIFT_FLAGS_GEN_OPT} ${THRIFT_FLAGS_LANGUAGE} ${THRIFT_FLAGS_OUTPUT} ${THRIFT_FLAGS_OUTPUT_DIRECTORY} ${THRIFT_FILE} RESULT_VARIABLE result OUTPUT_VARIABLE output)

		IF(${result} EQUAL 0)
			MESSAGE(STATUS "Thrift code generated")
		ELSE(${result} EQUAL 0)
			MESSAGE(SEND_ERROR "Cannot generate thrift code")
		ENDIF(${result} EQUAL 0)

	ELSE(EXISTS ${THRIFT_FILE})
		MESSAGE(SEND_ERROR "Cannot find the thrift file to generate")
	ENDIF(EXISTS ${THRIFT_FILE})
ELSE(THRIFT_BIN)
	MESSAGE(SEND_ERROR "Cannot find thrift in your path.")
ENDIF(THRIFT_BIN)

#
# Patch stuff
#
IF(PATCH_BIN)
	MESSAGE(STATUS "Patch command found")
ELSE(PATCH_BIN)
	MESSAGE(SEND_ERROR "Cannot find any patch command")
ENDIF(PATCH_BIN)

SET(PATCH_P_FLAG "-p0")
SET(PATCH_T_FLAG "-t")
SET(PATCH_N_FLAG "-N")
SET(PATCH_FILE_FLAG "${CMAKE_CURRENT_LIST_DIR}/src/gen-cpp/ows_rpc.h")
SET(PATCH_PATCH_FLAG "${CMAKE_CURRENT_LIST_DIR}/patches/platform.patch")

MESSAGE(STATUS "${PATCH_BIN} ${PATCH_P_FLAG} ${PATCH_T_FLAG} ${PATCH_N_FLAG} ${PATCH_FILE_FLAG} ${PATCH_PATCH_FLAG}")

#
# External project
#
ExternalProject_Add(
	thrift
	SOURCE_DIR	${THRIFT_GEN_CPP}
	PATCH_COMMAND	${PATCH_BIN} ${PATCH_P_FLAG} ${PATCH_T_FLAG} ${PATCH_N_FLAG} ${PATCH_FILE_FLAG} ${PATCH_PATCH_FLAG}
	BUILD_COMMAND	${THRIFT_BIN} ${THRIFT_FLAGS_RECURSIVE} ${THRIFT_FLAGS_GEN_OPT} ${THRIFT_FLAGS_LANGUAGE} ${THRIFT_FLAGS_OUTPUT} ${THRIFT_FLAGS_OUTPUT_DIRECTORY} ${THRIFT_FILE}
	INSTALL_COMMAND	${FAKE_INSTALL_BIN}
)
# see configure_file to patch
# http://stackoverflow.com/questions/1438535/how-to-run-a-command-at-compile-with-in-makefile-generated-by-cmake
# http://www.kitware.com/products/html/BuildingExternalProjectsWithCMake2.8.html0

add_executable(master
	src/master.cpp
	src/common.cpp
	src/config.cpp
	src/database.cpp
	src/domain.cpp
	src/job.cpp
	src/node.cpp
	src/router.cpp
	src/rpc_client.cpp
	src/rpc_server.cpp
	src/gen-cpp/ows_rpc.cpp
	src/gen-cpp/model_constants.cpp
	src/gen-cpp/model_types.cpp

	src/common.h
	src/config.h
	src/database.h
	src/day.h
	src/domain.h
	src/job.h
	src/node.h
	src/router.h
	src/rpc_client.h
	src/rpc_server.h
	src/gen-cpp/model_constants.h
	src/gen-cpp/model_types.h
	src/gen-cpp/ows_rpc.h
)

add_executable(client
	src/client.cpp
	src/common.cpp
	src/config.cpp
	src/database.cpp
	src/domain.cpp
	src/job.cpp
	src/node.cpp
	src/router.cpp
	src/rpc_client.cpp
	src/rpc_server.cpp
	src/gen-cpp/ows_rpc.cpp
	src/gen-cpp/model_constants.cpp
	src/gen-cpp/model_types.cpp

	src/common.h
	src/config.h
	src/database.h
	src/day.h
	src/domain.h
	src/job.h
	src/node.h
	src/router.h
	src/rpc_client.h
	src/rpc_server.h
	src/gen-cpp/model_constants.h
	src/gen-cpp/model_types.h
	src/gen-cpp/ows_rpc.h
)

add_executable(shell
	src/shell.cpp
	src/common.cpp
	src/config.cpp
	src/node.cpp
	src/rpc_client.cpp
	src/gen-cpp/ows_rpc.cpp
	src/gen-cpp/model_constants.cpp
	src/gen-cpp/model_types.cpp

	src/shell.h
	src/common.h
	src/config.h
	src/node.h
	src/rpc_client.h
	src/gen-cpp/model_constants.h
	src/gen-cpp/model_types.h
	src/gen-cpp/ows_rpc.h
)

add_subdirectory(src)
add_subdirectory(src/gen-cpp)
add_subdirectory(etc)

#find_package(Boost COMPONENTS regex thread FATAL_ERROR)

# Common link_directories
link_directories (
	/opt/local/lib
	/opt/local/lib/mysql
	/usr/lib
	/usr/lib/mysql

	/Users/mathieu/Developpements/c/libcli
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
link_directories (
	/Developer/SDKs/MacOSX10.6.sdk/usr/lib
)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Common include directories
include_directories(
	/opt/local/include
	/opt/local/include/mysql5
	/opt/local/include/mysql5/mysql
	/opt/local/include/thrift

	/usr/include
	/usr/include/mysql
	/usr/include/thrift

	/Users/mathieu/Developpements/c/libcli
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
include_directories(
	/Developer/SDKs/MacOSX10.6.sdk/usr/include
)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# targets
target_link_libraries(client
	#crypto
	#curl
	#dl
	pthread
	/opt/local/lib/libsqlite3.a
	#ssl
	#z

	/opt/local/lib/libidn.a
	/opt/local/lib/libthrift.a
	/opt/local/lib/libboost_regex.a
	/opt/local/lib/libboost_thread-mt.a
	/opt/local/lib/libboost_filesystem.a
	/opt/local/lib/mysql5/mysql/libmysqld.a
)

target_link_libraries(master
	#crypto
	#curl
	#dl
	pthread
	/opt/local/lib/libsqlite3.a
	#ssl
	#z

	/opt/local/lib/libthrift.a
	/opt/local/lib/libboost_regex.a
	/opt/local/lib/libboost_thread-mt.a
	/opt/local/lib/libboost_filesystem.a
	/opt/local/lib/mysql5/mysql/libmysqld.a
)

target_link_libraries(shell
	#crypto
	#curl
	#dl
	pthread
	#ssl
	#z

	/opt/local/lib/libidn.a
	/opt/local/lib/libthrift.a
	/opt/local/lib/libboost_regex.a
	/opt/local/lib/libboost_thread-mt.a
	/opt/local/lib/libboost_filesystem.a

	/Users/mathieu/Developpements/c/libcli/libcli.so
)
