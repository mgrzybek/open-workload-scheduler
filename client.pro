QT	-= core gui
CONFIG	+= link_pkgconfig debug_and_release

TARGET = client

include(qmake_conf/linux.pro)
include(qmake_conf/macx.pro)
include(qmake_conf/bsd.pro)
#include(qmake_conf/windows.pro)

INCLUDEPATH	+= include \
	src/gen-cpp

SOURCES += \
	src/convertions.cpp \
	src/cfg.cpp \
	src/database.cpp \
	src/domain.cpp \
	src/job.cpp \
	src/node.cpp \
	src/router.cpp \
	src/rpc_client.cpp \
	src/rpc_server.cpp \
	src/gen-cpp/model_constants.cpp \
	src/gen-cpp/model_types.cpp \
	src/gen-cpp/ows_rpc.cpp \
	src/client.cpp

HEADERS	+= include/common.h \
	include/convertions.h \
	include/cfg.h \
	include/database.h \
	include/domain.h \
	include/job.h \
	include/node.h \
	include/router.h \
	include/rpc_client.h \
	include/rpc_server.h \
	src/gen-cpp/model_constants.h \
	src/gen-cpp/model_types.h \
	src/gen-cpp/ows_rpc.h
