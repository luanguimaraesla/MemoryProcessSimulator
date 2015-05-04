all:	MMS.c
	gcc -ansi -Wall -pedantic -pthread -o simulator MMS.c

run:
	./simulator

clean:
	rm -rf *~ simulator
