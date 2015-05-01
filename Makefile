all:	MMS.c
	gcc -ansi -Wall -pedantic -o simulator MMS.c

run:
	./simulator

clean:
	rm -rf *~ simulator
