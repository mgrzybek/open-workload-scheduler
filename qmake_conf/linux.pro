linux {
	LIBS += -lthrift \
		-L/usr/local/lib \
		/usr/local/lib/mysql/libmysqld.a \
		-L/usr/lib \
		-L/usr/local/lib \
		/usr/local/lib/libboost_regex.a \
		/usr/local/lib/libboost_thread-mt.a \
		/usr/local/lib/libboost_filesystem.a \
		-lssl \
		-lpthread \
		-lcrypto \
		-lm \
		-ldl \
		-lz
	
	INCLUDEPATH +=  /usr/local/include \
		/usr/local/include/mysql \
		/usr/local/include/thrift \
		src
}

