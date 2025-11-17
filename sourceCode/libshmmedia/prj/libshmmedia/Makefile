
#!/bin/bash

######################################
#   lotus to build player on cygwin
######################################
PRJ_PATH=$(shell pwd)

BUILD_PATH=$(PRJ_PATH)/../../build
include $(BUILD_PATH)/Makefile.flat


VISHM_M?=1

DEF_MACROS=
LIBVAITEM_SHARED_MEMORY_PATH=$(PRJ_PATH)/../libvaItemSharedMemory
ifeq ($(VISHM_M), 1)
DEF_MACROS+=-D_LIBSHM_VARIABLE_ITEM_M
INCLUDS_PATHS+=-I$(LIBVAITEM_SHARED_MEMORY_PATH)/include -I$(LIBVAITEM_SHARED_MEMORY_PATH)/src/include
endif

DEP_LIBSHMMEDIA_PROTO_PATH=$(PRJ_PATH)/../libshmmediaProto
DEP_LIBSHMUTIL_PATH=$(PRJ_PATH)/../libshmUtil

PRJ_OBJ=$(PRJ_PATH)/obj
SUF_SRC=.cpp
INCLUDS_PATHS+=-I$(PRJ_PATH)/include -I$(DEP_LIBSHMMEDIA_PROTO_PATH)/include -I$(DEP_LIBSHMMEDIA_PROTO_PATH)/src \
-I$(DEP_LIBSHMUTIL_PATH)/include -I$(DEP_LIBSHMUTIL_PATH)/src/include \
-I$(PRJ_PATH)/../libsharememory/include \
-I$(PRJ_PATH)/../libsharememory/src/include \
-I$(PRJ_PATH)/../libtvufourcc/include

CFLAGS+=$(DEF_MACROS) $(INCLUDS_PATHS)

LDFLAGS+= -Llib -L$(PRJ_PATH)/../libshmmedia_protocol/lib -L$(PRJ_PATH)/../libsharememory/lib -L$(DEP_LIBSHMUTIL_PATH)/lib
LIBS+=-lshmmedia -lshmmediaProto -lsharememory
ifeq ($(VISHM_M), 1)
LDFLAGS+=-L$(LIBVAITEM_SHARED_MEMORY_PATH)/lib
LIBS+=-lvaItemSharedMemory
endif
LIBS+=-lshmUtil -pthread -lrt

SRCS=$(wildcard $(PRJ_PATH)/src/*$(SUF_SRC))
OBJS=$(patsubst %$(SUF_SRC), %.o, $(SRCS))
TEST_SRCS=$(wildcard $(PRJ_PATH)/test_libshmmedia/test$(SUF_SRC))
TEST_OBJS=$(patsubst %$(SUF_SRC), %.o, $(TEST_SRCS))
TEST_EXE=test_shmmedia
LIB_PATH=$(PRJ_PATH)/lib
LIB_AR_PRJ=lib/libshmmedia.a

DEPS=$(OBJS:.o=.d)
DEPFLAGS = -MT $@ -MM -MP -MF $*.d

INSTALL_PATH=$(BUILD_PATH)/libshmmedia

all:$(LIB_AR_PRJ)
	@#echo $(OBJS)

test:$(TEST_EXE)
	@

$(LIB_AR_PRJ):$(OBJS) 
	mkdir -p $(LIB_PATH)
	$(AR) -rcs $@ $(OBJS)

$(OBJS):%.o:%$(SUF_SRC) %.d
	$(CXX) -o $@ $(CFLAGS) -c $<

$(TEST_OBJS):%.o:%$(SUF_SRC)
	$(CXX) -o $@ $(CFLAGS) -c $^

$(TEST_EXE):$(TEST_OBJS) $(LIB_AR_PRJ)
	$(CXX) -o $@ $< $(LDFLAGS) $(LIBS)

$(DEPS):%.d:%$(SUF_SRC)
	$(CXX) $(CFLAGS) -E $< $(DEPFLAGS)

include $(wildcard $(DEPS))

clean:
	$(RM) $(OBJS) $(DEPS) $(TEST_OBJS) $(LIB_AR_PRJ) $(TEST_EXE)

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

