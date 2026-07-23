CC = gcc
CFLAGS = -Wall -Wextra
LDLIBS = -lm

SRCS = gate.c circuit.c qubit.c simulator.c run.c convertor.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean debug run

all: run

myproj: $(OBJS)

	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

run: myproj
	./myproj

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJS) myproj

debug: 
