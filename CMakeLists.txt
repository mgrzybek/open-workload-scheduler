cmake_minimum_required(VERSION 2.8)

project(ows)

add_definitions(-Wall -g -O2)
set(CMAKE_CXX_FLAGS_RELEASE)

add_executable(master src/master.cpp
	src/common.cpp
	src/config.cpp
	src/database.cpp
	src/domain.cpp
	src/job.cpp
	src/node.cpp
	src/router.cpp
	src/rpc_client.cpp
	src/rpc_server.cpp
	src/thrift/ows_rpc.cpp
	src/thrift/model_constants.cpp
	src/thrift/model_types.cpp

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
	src/thrift/model_constants.h
	src/thrift/model_types.h
	src/thrift/ows_rpc.h
)

add_executable(client src/client.cpp
	src/common.cpp
	src/config.cpp
	src/database.cpp
	src/domain.cpp
	src/job.cpp
	src/node.cpp
	src/router.cpp
	src/rpc_client.cpp
	src/rpc_server.cpp
	src/thrift/ows_rpc.cpp
	src/thrift/model_constants.cpp
	src/thrift/model_types.cpp

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
	src/thrift/model_constants.h
	src/thrift/model_types.h
	src/thrift/ows_rpc.h
)

add_executable(shell src/shell.cpp
	src/common.cpp
	src/config.cpp
	src/node.cpp
	src/rpc_client.cpp
	src/thrift/ows_rpc.cpp
	src/thrift/model_constants.cpp
	src/thrift/model_types.cpp

	src/shell.h
	src/common.h
	src/config.h
	src/node.h
	src/rpc_client.h
	src/thrift/model_constants.h
	src/thrift/model_types.h
	src/thrift/ows_rpc.h
)

add_subdirectory(src)
add_subdirectory(src/thrift)
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

	/Users/mathieu/Developpements/c/libcli
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
include_directories(
	/Developer/SDKs/MacOSX10.6.sdk/usr/include
)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# targets
target_link_libraries(client
	crypto
	curl
	dl
	pthread
	sqlite3
	ssl
	z

	/opt/local/lib/libidn.a
	/opt/local/lib/libthrift.a
	/opt/local/lib/libboost_regex.a
	/opt/local/lib/libboost_thread-mt.a
	/opt/local/lib/libboost_filesystem.a
	/opt/local/lib/mysql5/mysql/libmysqld.a
)

target_link_libraries(master
	crypto
	curl
	dl
	pthread
	sqlite3
	ssl
	z

	/opt/local/lib/libidn.a
	/opt/local/lib/libthrift.a
	/opt/local/lib/libboost_regex.a
	/opt/local/lib/libboost_thread-mt.a
	/opt/local/lib/libboost_filesystem.a
	/opt/local/lib/mysql5/mysql/libmysqld.a
)

target_link_libraries(shell
	crypto
	curl
	dl
	pthread
	ssl
	z

	/opt/local/lib/libidn.a
	/opt/local/lib/libthrift.a
	/opt/local/lib/libboost_regex.a
	/opt/local/lib/libboost_thread-mt.a
	/opt/local/lib/libboost_filesystem.a

	/Users/mathieu/Developpements/c/libcli/libcli.so
)
