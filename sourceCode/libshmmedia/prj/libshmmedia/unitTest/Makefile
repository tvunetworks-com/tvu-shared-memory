PRJ_PATH := $(shell pwd)/..

CXX ?= g++
CXXFLAGS ?= -g -DTVU_LINUX -std=c++11 -I$(PRJ_PATH)/include -I$(PRJ_PATH)/src \
-I$(PRJ_PATH)/../libsharememory/include -I$(PRJ_PATH)/../libsharememory/src -I$(PRJ_PATH)/../libsharememory/src/include \
-I$(PRJ_PATH)/../libshmmediaProto/include -I$(PRJ_PATH)/../libshmmediaProto/src -I$(PRJ_PATH)/../libshmmediaProto/src/include\
-I$(PRJ_PATH)/../libshmUtil/src/include -Wall -Wextra


LDFLAGS ?= -L$(PRJ_PATH)/lib \
-L$(PRJ_PATH)/../libsharememory/lib \
-L$(PRJ_PATH)/../libshmmediaProto/lib \
-L$(PRJ_PATH)/../libshmUtil/lib \
-L$(PRJ_PATH)/../libvaItemSharedMemory/lib \
-lshmmedia -lshmmediaProto -lsharememory -lvaItemSharedMemory -lshmUtil -lgtest -lgcov -lpthread -lz -lrt

TARGET := gtest_libshmmedia
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean lib

all: lib $(TARGET)

lib:
	$(MAKE) -C $(PRJ_PATH)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJS) $(TARGET)
