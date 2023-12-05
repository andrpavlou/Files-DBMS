ht:
	@echo " Compile ht_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./hash_functions/functions.c ./src/hash_file.c -lbf -o ./build/runner -O2 -lm -g

bf:
	@echo " Compile bf_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c -lbf -o ./build/runner -O2

run:
	rm data.db
	clear
	./build/runner

	
valgrind:
	valgrind --leak-check=yes ./build/runner 