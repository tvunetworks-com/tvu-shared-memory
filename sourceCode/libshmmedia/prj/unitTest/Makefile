PRJ_PATH := $(shell pwd)/..
BUILD_DIR := $(shell pwd)

#include $(PRJ_PATH)/../build/Makefile.flat

CXX ?= g++
CXXFLAGS += -g -std=c++11 -Wall -Wextra -DTVU_LINUX

PRJ1=${PRJ_PATH}/libshmUtil
PRJ2=${PRJ_PATH}/libsharememory
PRJ3=${PRJ_PATH}/libvaItemSharedMemory
PRJ4=${PRJ_PATH}/libshmmediaProto
PRJ5=${PRJ_PATH}/libshmmedia

CXXFLAGS += -DGTEST_MAIN_ENTRANCE=1 \
	-I$(PRJ1)/include \
	-I$(PRJ1)/src/include \
	-I$(PRJ1)/src \
	-I$(PRJ2)/include \
	-I$(PRJ2)/src/include \
	-I$(PRJ2)/src \
	-I$(PRJ3)/include \
	-I$(PRJ4)/include \
	-I$(PRJ4)/src/include \
	-I$(PRJ4)/src \
	-I$(PRJ5)/include \
	-I$(PRJ5)/src \

LIBS ?= -lshmmedia -lshmmediaProto -lsharememory -lvaItemSharedMemory -lshmUtil -lgcov

LIBS += -lgtest -lpthread -lrt -lz

LDFLAGS += \
	-L$(PRJ1)/lib \
	-L$(PRJ2)/lib \
	-L$(PRJ3)/lib \
	-L$(PRJ4)/lib \
	-L$(PRJ5)/lib \
	$(LIBS)


TARGET := unitTest
SRCS := $(wildcard src/*.cpp)
SRCS += $(wildcard $(PRJ1)/unitTest/src/*.cpp)
SRCS += $(wildcard $(PRJ2)/unitTest/src/*.cpp)
SRCS += $(wildcard $(PRJ3)/unitTest/src/*.cpp)
SRCS += $(wildcard $(PRJ4)/unitTest/src/*.cpp)
SRCS += $(wildcard $(PRJ5)/unitTest/src/*.cpp)
SRCS += $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJS) $(TARGET)

