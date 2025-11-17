#!/bin/bash

######################################
#   lotus compile test_libshmmedia
######################################
PRJ_PATH=$(shell pwd)
PRJ_OBJ=$(PRJ_PATH)/obj
LIBSHAREMEMORY=$(PRJ_PATH)/../../../libsharememory
SUF_SRC=.cpp
CFLAGS+= -g -fPIC -DTVU_LINUX -I$(PRJ_PATH)/../include -I$(LIBSHAREMEMORY)/src/include
LDFLAGS+= -L$(LIBSHAREMEMORY)/lib -L$(PRJ_PATH)/../lib -lsharememory -pthread -lrt

TEST_SRCS=$(wildcard $(PRJ_PATH)/*$(SUF_SRC))
TEST_OBJS=$(patsubst %$(SUF_SRC), %.o, $(TEST_SRCS))
TEST_EXE=$(patsubst %$(SUF_SRC), %, $(TEST_SRCS))

all:$(TEST_EXE)
	@#echo $(TEST_OBJS)

$(TEST_OBJS):%.o:%$(SUF_SRC)
	$(CXX) -o $@ $(CFLAGS) -c $^

$(TEST_EXE):%:%.o $(LIB_AR_PRJ)
	$(CXX) -o $@ $< $(LDFLAGS)

clean:
	$(RM) $(TEST_OBJS) $(TEST_EXE)

install:
	@
uninstall:
	@

.PHONY:all clean install uninstall

