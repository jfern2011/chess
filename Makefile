# Assign the compiler that we want to use:
CC=g++

# Make command
MAKE=make

#----------------------------------------------------------------------
# Include directories:
#----------------------------------------------------------------------
IDIRS = ../../abort/abort ../../signal/signal ../../types/types \
        ../../util/util ../../WriteEventSink/Write-Event-Sink/include/

# Object file directory:
ODIR=../obj

# Assign compiler options:
CFLAGS=-c -g -Wall -Wall -Wno-unused-function \
					$(foreach dir, $(IDIRS), -I$(dir)) --std=c++11

#----------------------------------------------------------------------
# Dynamic library dependencies:
#----------------------------------------------------------------------
LIBS=WriteEventSink
LD_PATH = ../../WriteEventSink/Write-Event-Sink/lib/

#----------------------------------------------------------------------
# Header dependencies:
#----------------------------------------------------------------------
_DEPS = abort.h Signal.h types.h util.h WriteEventSink.h

DEPS = $(join $(addsuffix /, $(IDIRS)), $(_DEPS))

#----------------------------------------------------------------------
# Object files:
#----------------------------------------------------------------------
_OBJ = StateMachine.o cmd.o cmd_ut.o
OBJ  = $(patsubst %, $(ODIR)/%, $(_OBJ))

# Compile WriteEventSink
WES_DIR = ../../WriteEventSink/Write-Event-Sink/src/
wes:
	@ cd $(WES_DIR) && $(MAKE) shared

# Generate object files
$(ODIR)/%.o: %.cpp $(DEPS)
	@ if ! [ -d $(ODIR) ]; then mkdir $(ODIR); fi
	$(CC) -c -o $@ $< $(CFLAGS)

# Build the unit test
test: wes $(OBJ)
	$(CC) -L$(LD_PATH) -Wl,-rpath=$(LD_PATH) -g -o $@ $(filter-out $<,$^) -l$(LIBS)

clean:
	@ rm $(ODIR)/*.o test

# This target is always out-of-date
.PHONY: clean++

clean++:
	@ cd $(WES_DIR) && $(MAKE) clean++
	@ rm -rf $(ODIR) *~ core $(IDIR)/*~ test
	@ echo clean++: all clean!
