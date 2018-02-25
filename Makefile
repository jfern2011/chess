# Assign the compiler that we want to use:
CC=g++

# Make command
MAKE=make

#----------------------------------------------------------------------
# Include directories:
#----------------------------------------------------------------------
IDIRS = abort/ signal/ types/ util/

# Object file directory:
ODIR=obj

# Assign compiler options:
CFLAGS=-c --std=c++11 -Wall \
			$(foreach dir, $(IDIRS), -I$(dir))

#----------------------------------------------------------------------
# Dynamic library dependencies:
#----------------------------------------------------------------------


#----------------------------------------------------------------------
# Header dependencies:
#----------------------------------------------------------------------
_DEPS = abort.h Signal.h types.h util.h

DEPS = $(join $(addsuffix /, $(IDIRS)), $(_DEPS))

#----------------------------------------------------------------------
# Object files:
#
# TODO: Move dependency generation to their respective folders
#----------------------------------------------------------------------

RES_IDIR = ReadEventSink/include/
RES_DEPS = $(RES_IDIR)/ReadEventSink.h
RES_ODIR = ReadEventSink/obj
RES_OBJ  = $(RES_ODIR)/ReadEventSink.o

CMD_IDIR = CommandLine/
CMD_DEPS = $(CMD_IDIR)/CommandLine.h
CMD_ODIR = CommandLine/obj
CMD_OBJ  = $(CMD_ODIR)/CommandLine.o

CHESS_DEPS = Buffer.h      \
             chess.h       \
             chess_util.h  \
             clock.h

CHESS_SRC = chess.cpp         \
            cmd.cpp           \
            DataTables.cpp    \
            engine.cpp        \
            EngineInputs.cpp  \
            EngineOutputs.cpp \
            log.cpp           \
            main.cpp          \
            movegen.cpp       \
            output2.cpp       \
            position.cpp      \
            protocol.cpp      \
            search.cpp        \
            StateMachine3.cpp

CHESS_H   = chess.h           \
			cmd.h             \
			DataTables.h      \
			engine.h          \
			EngineInputs.h    \
			EngineOutputs.h   \
			log.h             \
			movegen2.h        \
			output2.h         \
			position2.h       \
			protocol2.h       \
			search2.h         \
			StateMachine3.h

CHESS_OBJ  = $(patsubst %.cpp, $(ODIR)/%.o, $(CHESS_SRC))

$(RES_OBJ): ReadEventSink/src/ReadEventSink.cpp $(DEPS) $(RES_DEPS)
	@ if ! [ -d $(RES_ODIR) ]; then mkdir $(RES_ODIR); fi
	$(CC) -g -o $@ $< $(CFLAGS) -I$(RES_IDIR)

$(CMD_OBJ): CommandLine/CommandLine.cpp $(DEPS) $(CMD_DEPS)
	@ if ! [ -d $(CMD_ODIR) ]; then mkdir $(CMD_ODIR); fi
	$(CC) -g -o $@ $< $(CFLAGS) -I$(CMD_IDIR)

$(ODIR)/%.o: %.cpp
	@ if ! [ -d $(ODIR) ]; then mkdir $(ODIR); fi
	$(CC) -g -o $@ $< $(CFLAGS) -I$(RES_IDIR) -I$(CMD_IDIR)

engine: $(CHESS_H) $(CHESS_OBJ) $(RES_OBJ) $(CMD_OBJ)
	$(CC) -g -o $@ $(CHESS_OBJ) $(RES_OBJ) $(CMD_OBJ)

clean:
	@ rm -f $(RES_ODIR)/*.o
	@ rm -f $(CMD_ODIR)/*.o
	@ rm -f $(ODIR)/*.o

# This target is always out-of-date
.PHONY: clean++

clean++:
	@ rm -rf $(ODIR) $(RES_ODIR) $(CMD_ODIR) *~ core $(IDIR)/*~ engine
	@ echo clean++: all clean!
