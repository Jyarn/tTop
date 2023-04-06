CC=gcc
CFLAGS=-g -Wall
OUT=ttop.out
OBJ=main.o cpuPoll.o misc.o sessPoll.o memPoll.o IPC.o sysPoll.o
RARGS=--sequential

run: build
	./$(OUT) $(RARGS)

gdb: build
	gdb --args ./$(OUT) $(RARGS)

val: build
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all --trace-children=yes -s ./$(OUT) $(RARGS)

test: test.o IPC.o cpuPoll.o misc.o
	$(CC) test.o IPC.o cpuPoll.o misc.o -o $(OUT)

build: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ)
