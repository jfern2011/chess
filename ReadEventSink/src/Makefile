# Assign the compiler that we want to use:
CC=g++

#----------------------------------------------------------------------
# Include directories:
#----------------------------------------------------------------------
IDIR_BASE = ../../..
IDIRS = ../include $(IDIR_BASE)/abort/abort $(IDIR_BASE)/signal/signal \
        $(IDIR_BASE)/types/types $(IDIR_BASE)/util/util

# Object file directory:
ODIR=../obj

# Library directory:
LDIR=../lib

# Assign compiler options:
CFLAGS=-c -g -Wall -Wno-unused-function \
        			$(foreach dir, $(IDIRS), -I$(dir)) --std=c++11

LFLAGS=-c -fpic -Wall -Wno-unused-function \
					$(foreach dir, $(IDIRS), -I$(dir)) --std=c++11

#----------------------------------------------------------------------
# Header dependencies:
#----------------------------------------------------------------------
_DEPS = ReadEventSink.h abort.h Signal.h types.h util.h

DEPS = $(join $(addsuffix /, $(IDIRS)), $(_DEPS))

#----------------------------------------------------------------------
# Object files:
#----------------------------------------------------------------------
_OBJ = ReadEventSink.o ReadEventSink_ut.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

$(ODIR)/ReadEventSink.o: ReadEventSink.cpp $(DEPS)
	@ if ! [ -d $(ODIR) ]; then mkdir $(ODIR); fi
	$(CC) -g -o $@ $< $(LFLAGS)

# Generate object files
$(ODIR)/%.o: %.cpp $(DEPS)
	@ if ! [ -d $(ODIR) ]; then mkdir $(ODIR); fi
	$(CC) -c -g -o $@ $< $(CFLAGS)

# Make a shared object
shared: $(ODIR)/ReadEventSink.o
	@ if ! [ -d $(LDIR) ]; then mkdir $(LDIR); fi
	$(CC) -shared -o $(LDIR)/libWriteEventSink.so $^

# Build unit test
test: $(OBJ)
	$(CC) -g -D CONSOLE_TEST -o $@ $^

clean:
	@ rm $(ODIR)/*.o test

# This target is always out-of-date
.PHONY: clean++

clean++:
	@ rm -rf $(ODIR) $(LDIR) *~ core $(IDIR)/*~ test
	@ echo clean++: all clean!
