CC = gcc
CFLAGS = -Wall -Wextra
LDLIBS = -lm
CURSES_LIBS ?= -lncurses

CORE_OBJS = gate.o circuit.o qubit.o simulator.o convertor.o workflow.o
OBJS = $(CORE_OBJS) run.o tui.o tests/test_editor.o
DEPS = $(OBJS:.o=.d)

.PHONY: all clean debug run test

all: run

myproj: $(CORE_OBJS) run.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

editor: $(CORE_OBJS) tui.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(CURSES_LIBS) $(LDLIBS)

tests/test_editor: $(CORE_OBJS) tests/test_editor.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

test: tests/test_editor
	./tests/test_editor

run: myproj
	./myproj

%.o: %.c makefile
	$(CC) $(CPPFLAGS) -I. $(CFLAGS) -MMD -MP -c $< -o $@


clean:
	rm -f $(OBJS) $(DEPS) myproj editor tests/test_editor

debug: 

-include $(DEPS)
