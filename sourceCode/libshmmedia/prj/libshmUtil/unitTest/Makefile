PRJ_PATH := $(shell pwd)/..
BUILD_DIR := $(shell pwd)

CXX ?= g++
CXXFLAGS ?= -g -std=c++11 -I$(PRJ_PATH)/include -I$(PRJ_PATH)/src/include -Wall -Wextra
LDFLAGS ?= -L$(PRJ_PATH)/lib -lshmUtil -lgtest -lpthread -lz

TARGET := gtest_libshmUtil
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

