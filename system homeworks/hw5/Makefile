run: 
	gcc hw5.c queue.c -o hw5 -pthread
	./hw5 -i data.dat

run-wall: 
	gcc -Wall hw5.c queue.c -o hw5 -pthread
	./hw5 -i data.dat

compile:
	gcc hw5.c queue.c -o hw5 -pthread

compile-wall:
	gcc -Wall hw5.c queue.c -o hw5 -pthread

compile-debug:
	gcc -g hw5.c queue.c -o hw5 -pthread

valgrind: compile
	valgrind ./hw5 -i data.dat 

vv: compile
	valgrind -v ./hw5 -i data.dat
clean:
	rm -rf *.o hw5