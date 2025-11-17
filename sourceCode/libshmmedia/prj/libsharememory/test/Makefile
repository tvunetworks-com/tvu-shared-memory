
CFLAGS += -DTVU_LINUX -g -I../include
LDFLAGS += -L../lib -L../../libtvuutil/lib
LIBS=-lsharememory -ltvuutil

EXE1=shmmng_test

all:shmmng_test
	@

$(EXE1):shmmng_test.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) $(EXE1)

.PHONY:all clean install uninstall

