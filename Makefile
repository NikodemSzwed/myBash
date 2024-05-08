main: main.o
	gcc -o main main.o
main2: main2.o
	gcc -o main2 main2.o

.PHONY: clean
clean:
	rm -f main main2 *.o .myBash_history.txt