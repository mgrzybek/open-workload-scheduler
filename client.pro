QT	-= core gui
CONFIG	+= link_pkgconfig

TARGET = client

QMAKE_EXTRA_TARGET += src/gen-cpp/ows_rpc.cpp
#PRE_TARGETDEPS += thrift

include(qmake_conf/linux.pro)
include(qmake_conf/macx.pro)
include(qmake_conf/bsd.pro)
#include(qmake_conf/windows.pro)

SOURCES += src/client.cpp
	src/convertions.cpp \
	src/config.cpp \
	src/database.cpp \
	src/domain.cpp \
	src/job.cpp \
	src/node.cpp \
	src/router.cpp \
	src/rpc_client.cpp \
	src/rpc_server.cpp \
	src/gen-cpp/model_constants.cpp \
	src/gen-cpp/model_types.cpp \
	src/gen-cpp/ows_rpc.cpp

HEADERS	+= src/common.h \
	src/convertions.h \
	src/config.h \
	src/database.h \
	src/domain.h \
	src/job.h \
	src/node.h \
	src/router.h \
	src/rpc_client.h \
	src/rpc_server.h \
	src/gen-cpp/model_constants.h \
	src/gen-cpp/model_types.h \
	src/gen-cpp/ows_rpc.h
