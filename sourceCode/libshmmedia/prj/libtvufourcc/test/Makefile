#!/bin/bash

######################################
#   lotus compile test_libshmmedia
######################################
PRJ_PATH=$(shell pwd)
PRJ_OBJ=$(PRJ_PATH)/obj
SUF_SRC=.cpp
DEPEND_1=../../../../../3rdparty/libffmpeg/release/linux
CFLAGS+= -fPIC -DTVU_LINUX -I$(PRJ_PATH)/../include -I$(DEPEND_1)/include
LDFLAGS+= -L$(PRJ_PATH)/../lib -ltvufourcc

TEST_SRCS=$(wildcard $(PRJ_PATH)/*$(SUF_SRC))
TEST_OBJS=$(patsubst %$(SUF_SRC), %.o, $(TEST_SRCS))
TEST_EXE=test_libtvufourcc

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

