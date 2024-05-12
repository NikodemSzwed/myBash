myBash: main.o
	gcc -o myBash main.o

.PHONY: clean
clean:
	rm -f myBash *.o .myBash_history.txt