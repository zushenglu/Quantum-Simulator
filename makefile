CC = gcc
CFLAGS = -Wall -Wextra
LDLIBS = -lm
CURSES_LIBS ?= -lncurses

CORE_OBJS = gate.o circuit.o qubit.o simulator.o convertor.o workflow.o
OBJS = $(CORE_OBJS) run.o tui.o tests/test_editor.o
DEPS = $(OBJS:.o=.d)
BENCHMARKS = circuit_small test_circuit

.PHONY: all clean debug run test

all: test_circuit run

myproj: $(CORE_OBJS) run.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

circuit_small: BENCHMARK_CPPFLAGS = -DBENCHMARK_SMALL
circuit_small: run.c my_circuit.c
test_circuit: BENCHMARK_CPPFLAGS = -Dstoi=atoi
test_circuit: test_circuit.c

ifeq ($(shell uname -s),Darwin)
ifeq ($(origin CC),file)
test_circuit: CC = $(firstword $(wildcard /opt/homebrew/opt/llvm/bin/clang /usr/local/opt/llvm/bin/clang) clang)
test_circuit: LDFLAGS += --ld-path=/usr/bin/ld
endif
endif

$(BENCHMARKS): $(CORE_OBJS:.o=.c) $(wildcard *.h) makefile
	$(CC) $(CPPFLAGS) $(BENCHMARK_CPPFLAGS) $(CFLAGS) -O2 -fopenmp \
		$(LDFLAGS) -o $@ $(filter %.c,$^) $(LDLIBS)

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
	rm -f $(OBJS) $(DEPS) myproj editor tests/test_editor $(BENCHMARKS)

debug: 

-include $(DEPS)
