PRJ_PATH := $(shell pwd)/..

CXX ?= g++
CXXFLAGS ?= -DTVU_LINUX -std=c++11 -I$(PRJ_PATH)/include -I$(PRJ_PATH)/src -I$(PRJ_PATH)/../libshmUtil/src/include -Wall -Wextra
LDFLAGS ?= -L$(PRJ_PATH)/lib -L$(PRJ_PATH)/../libshmUtil/lib -lshmmediaProto -lshmUtil -lgtest -lpthread -lz

TARGET := gtest_libshmmediaProto
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
