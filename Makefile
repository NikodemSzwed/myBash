main: main.o
	gcc -o main main.o

.PHONY: clean
clean:
	rm -f main *.o