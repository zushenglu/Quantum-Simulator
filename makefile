
gate.o: gate.c
	gcc -c gate.c

circuit.c: gate.c
	gcc -c circuit.c

circuit.o: circuit.c
	gcc -c circuit.c

myproj: circuit.o, gate.o
	gcc -o myproj gate.o circuit.o

clean:
	rm -f gate.o circuit.o myproj

