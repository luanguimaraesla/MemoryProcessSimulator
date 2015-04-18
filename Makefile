all:	MPS.c
	gcc -ansi -Wall -pedantic -o simulator MPS.c

run:
	./simulator

clean:
	rm -rf *~ simulator
