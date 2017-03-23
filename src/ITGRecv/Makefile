OBJS = ../common/ITG.o ../common/timestamp.o ../common/serial.o ../common/pipes.o ../$(THOBJS) data.o ITGRecv.o neat_server.o

all: ITGRecv$(SUFFIX)

ITGRecv$(SUFFIX): ../common/ITG.h ITGRecv.o $(OBJS)
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@ echo [ LD ]  $@ ^<- $(OBJS)
else
	@ printf "[ LD ]\t$@ <- $(OBJS)\n"
endif
	@ $(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS) ../../neat/build/libneat.so /usr/lib/x86_64-linux-gnu/libuv.so
	@ $(CP) $@ $(BIN)

clean:
ifeq ($(BUILD_ENV),WIN32_MINGW)
	@- $(RM) *.o ITGRecv$(SUFFIX) $(BIN)\ITGRecv$(SUFFIX)
else
	@ $(RM) *.o ITGRecv$(SUFFIX) $(BIN)/ITGRecv$(SUFFIX)
endif

#---------------
include ../common.mk
#---------------
