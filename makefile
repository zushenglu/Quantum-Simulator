CC = gcc
CFLAGS = -Wall -Wextra

SRCS = gate.c circuit.c
OBJS = $(SRCS:.c=.o) 

all: myproj

myproj: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJS) myproj
