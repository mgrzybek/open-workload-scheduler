!macx:unix {
	LIBS += -lthrift \
		-L/usr/local/lib \
		-L/usr/local/lib/mysql \
		/usr/local/lib/mysql/libmysqld.a \
		-L/usr/lib \
		-L/usr/local/lib \
		/usr/local/lib/libboost_program_options.a \
		/usr/local/lib/libboost_regex.a \
		/usr/local/lib/libboost_thread.a \
		/usr/local/lib/libboost_filesystem.a \
		/usr/local/lib/libboost_system.a \
		/usr/local/lib/liblog4cpp.a \
		-lssl \
		-lpthread \
		-lcrypt \
		-lm \
		-lz

	INCLUDEPATH +=  /usr/local/include \
		/usr/local/include/mysql \
		/usr/local/include/thrift \
		src
}

