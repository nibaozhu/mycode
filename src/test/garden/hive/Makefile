all: hive

hive: hive.o hivep.pb.o
	$(CXX) -o $@ $^ $(LDFLAGS)

hive.o: hive.cc hivep.pb.h
hivep.pb.o: hivep.pb.cc hivep.pb.h

hivep.pb.cc hivep.pb.h: hivep.proto
	protoc --cpp_out=./ $^

.PHONY: all clean
clean:
	rm --force hive hive.o hivep.pb.* /tmp/hive.log* core.*

CPPFLAGS=-DHIVE_VERSION_STRING=\"1.0.0\"
# CXXFLAGS=-std=c++11 -Wall -Wno-pointer-arith -Wno-deprecated-declarations -Wno-unused-variable -Ofast
CXXFLAGS=-std=c++11 -Wall -Wno-pointer-arith -Wno-deprecated-declarations -Wno-unused-variable -O0 -g
LDFLAGS=-pthread -ldl -lunwind \
		/usr/local/lib/libtcmalloc.a \
		/usr/local/lib/libhiredis.a \
		/usr/local/lib/liblog4cplus.a \
		/usr/local/lib/libprotobuf.a \
		/usr/local/lib/libz.a \
		/usr/local/lib/libzmq.a \
		/usr/lib64/libevent.so \
		/usr/lib64/libevent_openssl.so \
		/usr/lib64/libssl.so \
		/usr/lib64/libcrypto.so \
		/usr/lib64/libnghttp2.so
