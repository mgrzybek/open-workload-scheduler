macx {
	PKGCONFIG += thrift
	
	LIBS += -lthrift \
		-L/opt/local/lib \
		#-L/opt/local/lib/mysql5/mysql \
		/opt/local/lib/mysql5/mysql/libmysqld.a \
		-L/usr/lib \
		-L/usr/local/lib \
		#/opt/local/lib/libboost_regex.a \
		#/opt/local/lib/libboost_thread-mt.a \
		-lssl \
		-lpthread \
		-lcrypto \
		-lm \
		-ldl \
		-lz \
		/Users/mathieu/Desktop/boost_1_49_0/stage/lib/libboost_regex.a \
		/Users/mathieu/Desktop/boost_1_49_0/stage/lib/libboost_thread.a
	
	INCLUDEPATH += /usr/include \
		/opt/local/include \
		/opt/local/include/mysql5/mysql \
		/opt/local/include/thrift \
		src \
		/Users/mathieu/Desktop/boost_1_49_0
}

