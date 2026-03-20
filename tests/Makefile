#!/bin/bash

######################################
#   Test suite for tvu-shared-memory
######################################

PRJ_PATH=$(shell pwd)
PRJ_OBJ=$(PRJ_PATH)/obj
UNIT_OBJ=$(PRJ_OBJ)/unit
INTEGRATION_OBJ=$(PRJ_OBJ)/integration

# Compiler flags
CXXFLAGS += -g -fPIC -DTVU_LINUX -std=c++11
CXXFLAGS += -I$(PRJ_PATH)/../sourceCode/libshmmedia/prj/libshmmedia/include
CXXFLAGS += -I$(PRJ_PATH)/../sourceCode/libshmmedia/prj/libshmmediaProto/include
CXXFLAGS += -I$(PRJ_PATH)/../sourceCode/libshmmedia/prj/libvaItemSharedMemory/include
CXXFLAGS += -I$(PRJ_PATH)/../include

# Linker flags
LDFLAGS += -L$(PRJ_PATH)/../lib -lshmmediawrap -pthread -lrt -lgtest -lgtest_main

# Create object directories
$(shell mkdir -p $(UNIT_OBJ))
$(shell mkdir -p $(INTEGRATION_OBJ))

# Unit test sources and executables
UNIT_SRCS = $(wildcard $(PRJ_PATH)/unit/*.cpp)
UNIT_OBJS = $(patsubst $(PRJ_PATH)/unit/%.cpp, $(UNIT_OBJ)/%.o, $(UNIT_SRCS))
UNIT_EXES = $(patsubst $(PRJ_PATH)/unit/%.cpp, $(PRJ_PATH)/unit/%, $(UNIT_SRCS))

# Integration test sources and executables
INTEGRATION_SRCS = $(wildcard $(PRJ_PATH)/integration/*.cpp)
INTEGRATION_OBJS = $(patsubst $(PRJ_PATH)/integration/%.cpp, $(INTEGRATION_OBJ)/%.o, $(INTEGRATION_SRCS))
INTEGRATION_EXES = $(patsubst $(PRJ_PATH)/integration/%.cpp, $(PRJ_PATH)/integration/%, $(INTEGRATION_SRCS))

# All executables
ALL_EXES = $(UNIT_EXES) $(INTEGRATION_EXES)

.PHONY: all clean test unit_tests integration_tests

all: $(ALL_EXES)

# Compile unit test objects
$(UNIT_OBJ)/%.o: $(PRJ_PATH)/unit/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link unit test executables
$(PRJ_PATH)/unit/%: $(UNIT_OBJ)/%.o
	$(CXX) -o $@ $< $(LDFLAGS)

# Compile integration test objects
$(INTEGRATION_OBJ)/%.o: $(PRJ_PATH)/integration/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link integration test executables
$(PRJ_PATH)/integration/%: $(INTEGRATION_OBJ)/%.o
	$(CXX) -o $@ $< $(LDFLAGS)

# Run all tests
test: all
	@echo "Running unit tests..."
	@for test in $(UNIT_EXES); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "Running integration tests..."
	@for test in $(INTEGRATION_EXES); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "All tests passed!"

# Run only unit tests
unit_tests: $(UNIT_EXES)
	@echo "Running unit tests..."
	@for test in $(UNIT_EXES); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "Unit tests passed!"

# Run only integration tests
integration_tests: $(INTEGRATION_EXES)
	@echo "Running integration tests..."
	@for test in $(INTEGRATION_EXES); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "Integration tests passed!"

clean:
	$(RM) -r $(PRJ_OBJ)
	$(RM) $(UNIT_EXES) $(INTEGRATION_EXES)

install:
	@

uninstall:
	@
