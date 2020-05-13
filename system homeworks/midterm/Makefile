run:compile 
	./exe -N 3 -M 12 -T 5 -S 4 -L 13 -F input.txt
compile: 
	gcc midterm.c -pthread -o exe
	 
compile-debug:
	gcc -g midterm.c -pthread -o exe

valgrind: compile
	valgrind ./exe -N 3 -M 12 -T 5 -S 4 -L 13 -F input.txt

vv: compile-debug
	valgrind -v ./exe -N 3 -M 12 -T 5 -S 4 -L 13 -F input.txt
clean:
	rm -rf *.o exe
