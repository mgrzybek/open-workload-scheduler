linux {
	LIBS += -lthrift \
		-L/usr/local/lib \
		/usr/lib/libmysqld.a \
		-L/usr/lib \
		-L/usr/local/lib \
		/usr/lib/libboost_regex.a \
		/usr/lib/libboost_thread.a \
		/usr/lib/libboost_filesystem.a \
		/usr/lib/libboost_system.a \
		-lssl \
		-lpthread \
		-lcrypto \
		-lm \
		-ldl \
		-lz

	INCLUDEPATH +=  /usr/include \
		/usr/include/mysql \
		/usr/include/thrift \
		src
}

