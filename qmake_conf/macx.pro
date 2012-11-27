macx {
#	PKGCONFIG += thrift libzmq

	LIBS += -lthrift \
		-lzmq \
		-L/opt/local/lib \
		/opt/local/lib/mysql55/mysql/libmysqlservices.a \
		-L/usr/lib \
		-L/usr/local/lib \
		/opt/local/lib/libboost_regex-mt.a \
		/opt/local/lib/libboost_thread-mt.a \
		/opt/local/lib/libboost_system-mt.a \
		-lssl \
		-lpthread \
		-lcrypto \
		-lm \
		-ldl \
		-lz

	INCLUDEPATH += /usr/include \
		/opt/local/include \
		/opt/local/include/mysql55/mysql \
		/opt/local/include/thrift \
		src
}

