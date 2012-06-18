
# Defines how to build Thrift sources
thrift.target = src/gen-cpp/ows_rpc.cpp
thrift.commands = thrift --gen cpp -o src src/model.thrift
thrift.depends = src/model.thrift

QMAKE_EXTRA_TARGET += src/gen-cpp/ows_rpc.cpp
TARGET = master
PRE_TARGETDEPS += thrift

unix:LIBS += -lthrift \
        -L/opt/local/lib \
        -L/opt/local/lib/mysql5/mysql \
	/opt/local/lib/mysql5/mysql/libmysqld.a \
        -L/usr/lib \
        -L/usr/local/lib \
        /opt/local/lib/libboost_regex.a \
        /opt/local/lib/libboost_thread-mt.a \
        /opt/local/lib/libboost_filesystem.a \
	-lssl \
	-lpthread \
	-lcrypto \
	-lm \
	-ldl \
	-lz

INCLUDEPATH +=  /opt/local/include \
        /opt/local/include/mysql \
        /opt/local/include/mysql5 \
        /opt/local/include/mysql5/mysql \
        /opt/local/include/thrift \
        src


SOURCES += src/common.cpp \
                src/config.cpp \
                src/database.cpp \
                src/domain.cpp \
                src/job.cpp \
                src/master.cpp \
                src/node.cpp \
                src/router.cpp \
                src/rpc_client.cpp \
                src/rpc_server.cpp \
                src/gen-cpp/model_constants.cpp \
                src/gen-cpp/model_types.cpp \
                src/gen-cpp/ows_rpc.cpp

HEADERS	+= src/common.h \
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

