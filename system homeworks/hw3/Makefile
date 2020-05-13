run: 
	gcc hw2.c -o hw2
	./hw2 -i input.txt -o output.txt

compile-debug:
	gcc -g hw2.c -o hw2

valgrind: compile
	valgrind ./hw2 -i input.txt -o output.txt

vv: compile-debug
	valgrind -v ./hw2 -i input.txt -o output.txt
clean:
	rm -rf *.o hw2 