CC = gcc
CFLAGS = -Wall -Wextra
LDLIBS = -lm

SRCS = gate.c circuit.c qubit.c simulator.c run.c convertor.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean debug run test

all: run

myproj: $(OBJS)

	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

run: myproj
	./myproj

tests/circuit_lifecycle: tests/circuit_lifecycle.c circuit.c circuit.h gate.c gate.h qubit.c qubit.h
	$(CC) $(CFLAGS) -I. -o $@ tests/circuit_lifecycle.c circuit.c gate.c qubit.c $(LDLIBS)

test: tests/circuit_lifecycle
	./tests/circuit_lifecycle

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJS) myproj tests/circuit_lifecycle

debug: 
