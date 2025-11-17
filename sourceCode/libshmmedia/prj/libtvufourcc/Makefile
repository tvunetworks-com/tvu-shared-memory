#!/bin/bash

######################################
#   lotus to build player on cygwin
######################################

PRJ_PATH=$(shell pwd)
BUILD_PATH=$(PRJ_PATH)/../../build
include $(BUILD_PATH)/Makefile.flat

PRJ_OBJ=$(PRJ_PATH)/obj
SUF_SRC=.cpp
DEPEND_1=../../../../3rdparty/libffmpeg/release/linux
CFLAGS+=-I$(PRJ_PATH)/include -I$(PRJ_PATH)/../../include -Isrc/include -I$(DEPEND_1)/include
LDFLAGS+=

SRCS=$(wildcard $(PRJ_PATH)/src/*$(SUF_SRC))
OBJS=$(patsubst %$(SUF_SRC), %.o, $(SRCS))
LIB_AR_PRJ=lib/libtvuMediaInfo.a

INSTALL_PATH=$(BUILD_PATH)/libshmmedia

all:$(LIB_AR_PRJ)
	@#echo $(OBJS)


$(LIB_AR_PRJ):$(OBJS) 
	$(AR) -rcs $@ $(OBJS)

$(OBJS):%.o:%$(SUF_SRC)
	$(CXX) -o $@ $(CFLAGS) -c $^

clean:
	$(RM) $(OBJS) $(LIB_AR_PRJ) 

install:
	@(mkdir -p $(INSTALL_PATH);\
	mkdir -p $(INSTALL_PATH)/include;\
	mkdir -p $(INSTALL_PATH)/lib/release;\
	cp -f $(PRJ_PATH)/lib/*.a $(INSTALL_PATH)/lib/release;\
	)

uninstall:
	@

.PHONY:all clean install uninstall

