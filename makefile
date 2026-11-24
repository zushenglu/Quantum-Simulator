CC = gcc
CFLAGS = -Wall -Wextra
LDLIBS = -lm

SRCS = gate.c circuit.c qubit.c simulator.c run.c convertor.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean debug run

all: run

myproj: $(OBJS)

	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

run: myproj
	./myproj

%.o: %.c makefile
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@


clean:
	rm -f $(OBJS) $(DEPS) myproj

debug: 

-include $(DEPS)
