
#!/bin/bash

######################################
#   lotus to build player on cygwin
######################################
PRJ_PATH=$(shell pwd)

BUILD_PATH=$(PRJ_PATH)/../../build
include $(BUILD_PATH)/Makefile.flat

DEF_MACROS=

PRJ_OBJ=$(PRJ_PATH)/obj
SUF_SRC=.cpp

DEP_LIBSHMUTIL_PATH=$(PRJ_PATH)/../libshmUtil

INCLUDS_PATHS+=-I$(PRJ_PATH)/include  -I$(PRJ_PATH)/../libsharememory/include -I$(PRJ_PATH)/../libsharememory/src/include -I$(DEP_LIBSHMUTIL_PATH)/include -I$(DEP_LIBSHMUTIL_PATH)/src/include
CFLAGS+=$(DEF_MACROS) $(INCLUDS_PATHS)

LDFLAGS+= -Llib -L$(PRJ_PATH)/../libsharememory/lib

SRCS=$(wildcard $(PRJ_PATH)/src/*$(SUF_SRC))
OBJS=$(patsubst %$(SUF_SRC), %.o, $(SRCS))
TEST_SRCS=$(wildcard $(PRJ_PATH)/test_libshmmedia/test$(SUF_SRC))
TEST_OBJS=$(patsubst %$(SUF_SRC), %.o, $(TEST_SRCS))
LIB_PATH=$(PRJ_PATH)/lib
LIB_AR_PRJ=lib/libshmmediaProto.a

DEPS=$(OBJS:.o=.d)
DEPFLAGS = -MT $@ -MM -MP -MF $*.d

INSTALL_PATH=$(BUILD_PATH)/libshmmedia

all:$(LIB_AR_PRJ)
	@#echo $(OBJS)

$(LIB_AR_PRJ):$(OBJS) 
	mkdir -p $(LIB_PATH)
	$(AR) -rcs $@ $(OBJS)

$(OBJS):%.o:%$(SUF_SRC) %.d
	$(CXX) -o $@ $(CFLAGS) -c $<

$(TEST_OBJS):%.o:%$(SUF_SRC)
	$(CXX) -o $@ $(CFLAGS) -c $^

$(DEPS):%.d:%$(SUF_SRC)
	$(CXX) $(CFLAGS) -E $< $(DEPFLAGS)

include $(wildcard $(DEPS))

clean:
	$(RM) $(OBJS) $(DEPS) $(TEST_OBJS) $(LIB_AR_PRJ)

install:
	@(mkdir -p $(INSTALL_PATH);\
	mkdir -p $(INSTALL_PATH)/include;\
	mkdir -p $(INSTALL_PATH)/lib/release;\
    cp -f $(PRJ_PATH)/include/*.h $(INSTALL_PATH)/include;\
	cp -f $(PRJ_PATH)/lib/*.a $(INSTALL_PATH)/lib/release;\
	)
uninstall:
	@

.PHONY:all clean install uninstall test

