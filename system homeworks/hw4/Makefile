run: 
	gcc -Wall hw4.c -o hw4 -pthread
	./hw4 -i input.txt
compile:
	gcc -Wall hw4.c -o hw4 -pthread

compile-debug:
	gcc -g hw4.c -o hw4 -pthread

valgrind: compile
	valgrind --leak-check=full ./hw4 -i input.txt 

vv: compile
	valgrind -v ./hw4 -i input.txt
clean:
	rm -rf *.o hw4 