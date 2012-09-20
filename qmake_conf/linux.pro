linux {
	# Defines how to build Thrift sources
	thrift.target = src/gen-cpp/ows_rpc.cpp
	thrift.commands = thrift --gen cpp -o src src/model.thrift
	thrift.depends = src/model.thrift
	
	#PRE_TARGETDEPS += thrift
	
	LIBS += -lthrift \
		-L/usr/local/lib \
		/usr/local/lib/mysql/libmysqld.a \
		-L/usr/lib \
		-L/usr/local/lib \
		#/usr/local/lib/libboost_regex.a \
		#/usr/local/lib/libboost_thread-mt.a \
		#/usr/local/lib/libboost_filesystem.a \
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

