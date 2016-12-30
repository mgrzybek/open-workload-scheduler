macx {
	LIBS += -lthrift \
		-L/opt/local/lib \
		/opt/local/lib/mariadb/mysql/libmysqld.a \
		-L/usr/lib \
		-L/usr/local/lib \
		/opt/local/lib/libboost_program_options-mt.a \
		/opt/local/lib/libboost_regex-mt.a \
		/opt/local/lib/libboost_thread-mt.a \
		/opt/local/lib/libboost_system-mt.a \
		/opt/local/lib/liblog4cpp.a \
		-lssl \
		-lpthread \
		-lcrypto \
		-lm \
		-ldl \
		-lz

	INCLUDEPATH += /usr/include \
		/opt/local/include \
		/opt/local/include/mariadb/mysql \
		/opt/local/include/thrift \
		src
}

