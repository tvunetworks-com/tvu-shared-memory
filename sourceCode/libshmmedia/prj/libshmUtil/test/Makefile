#!/bin/bash

######################################
#   lotus compile test_libshmmedia
######################################
PRJ_PATH=$(shell pwd)
PRJ_OBJ=$(PRJ_PATH)/obj
SUF_SRC=.cpp
DEP_1=$(PRJ_PATH)/../../libshmmediaProto
DEP_2=$(PRJ_PATH)/../../libvaItemSharedMemory
DEP_3=$(PRJ_PATH)/../../libshmUtil
CFLAGS+= -g -fPIC -DTVU_LINUX -I$(PRJ_PATH)/../include -I$(DEP_1)/include -I$(DEP_1)/src -I$(DEP_1)/src/include -I$(DEP_3)/include -I$(DEP_3)/src/include -I$(DEP_3)/src
LDFLAGS+= -L$(PRJ_PATH)/../lib -L$(DEP_1)/lib -L$(DEP_2)/lib -L$(DEP_3)/lib -L$(PRJ_PATH)/../../libsharememory/lib -lshmUtil -pthread -lrt -lz

TEST_SRCS=$(wildcard $(PRJ_PATH)/*$(SUF_SRC))
TEST_OBJS=$(patsubst %$(SUF_SRC), %.o, $(TEST_SRCS))
TEST_EXE=testUtil

all:$(TEST_EXE)
	@#echo $(TEST_OBJS)

$(TEST_OBJS):%.o:%$(SUF_SRC)
	$(CXX) -o $@ $(CFLAGS) -c $^

$(TEST_EXE):$(TEST_OBJS) $(LIB_AR_PRJ)
	$(CXX) -o $@ $< $(LDFLAGS)

clean:
	$(RM) $(TEST_OBJS) $(TEST_EXE)

install:
	@
uninstall:
	@

.PHONY:all clean install uninstall

