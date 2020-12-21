all: main.o parsing.o
	gcc -o bash_trash main.o parsing.o
main.o: main.c parsing.c parsing.h
	gcc -c main.c
parsing.o: parsing.c parsing.h
	gcc -c parsing.c
run:
	./bash_trash
clean:
	rm *.o