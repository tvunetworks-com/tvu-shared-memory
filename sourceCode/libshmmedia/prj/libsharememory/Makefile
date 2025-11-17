#!/bin/bash

######################################
#   lotus to build player on cygwin
######################################

PRJ_PATH=$(shell pwd)
BUILD_PATH=$(PRJ_PATH)/../../build
include $(BUILD_PATH)/Makefile.flat
REMOVE_SHM?=1
VISHM_M?=1

LIBVAITEM_SHARED_MEMORY_PATH=$(PRJ_PATH)/../libvaItemSharedMemory
DEF_MACROS+=-D_REMOVE_SHM_FROM_KEY=$(REMOVE_SHM)
ifeq ($(VISHM_M), 1)
DEF_MACROS+=-D_LIBSHM_VARIABLE_ITEM_M
INCLUDS_PATHS+=-I$(LIBVAITEM_SHARED_MEMORY_PATH)/include
endif

DEP_1=$(PRJ_PATH)/../libshmUtil
INCLUDS_PATHS+=-I$(PRJ_PATH)/include -I$(PRJ_PATH)/../../include -Isrc/include -I$(DEP_1)/include -I$(DEP_1)/src/include -I$(DEP_1)/src
PRJ_OBJ=$(PRJ_PATH)/obj
SUF_SRC=.cpp
CFLAGS+=$(DEF_MACROS) $(INCLUDS_PATHS)
LDFLAGS+=

SRCS=$(wildcard $(PRJ_PATH)/src/*$(SUF_SRC))
OBJS=$(patsubst %$(SUF_SRC), %.o, $(SRCS))
LIB_AR_PRJ=lib/libsharememory.a

DEPS=$(OBJS:.o=.d)
DEPFLAGS = -MT $@ -MM -MP -MF $*.d

INSTALL_PATH=$(BUILD_PATH)/libshmmedia

all:$(LIB_AR_PRJ)
	@#echo $(OBJS)

$(LIB_AR_PRJ):$(OBJS) 
	$(AR) -rcs $@ $(OBJS)

$(OBJS):%.o:%$(SUF_SRC) %.d
	$(CXX) -o $@ $(CFLAGS) -c $<

$(DEPS):%.d:%$(SUF_SRC)
	$(CXX) $(CFLAGS) -E $< $(DEPFLAGS)

include $(wildcard $(DEPS))
clean:
	$(RM) $(OBJS) $(DEPS) $(LIB_AR_PRJ) 

install:
	@(mkdir -p $(INSTALL_PATH);\
	mkdir -p $(INSTALL_PATH)/include;\
	mkdir -p $(INSTALL_PATH)/lib/release;\
	cp -f $(PRJ_PATH)/lib/*.a $(INSTALL_PATH)/lib/release;\
	)

uninstall:
	@

.PHONY:all clean install uninstall

