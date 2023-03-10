CC=gcc
CFLAGS=-g -Wall -Werror
OUT=ttop.out
OBJ=main.o cpuPoll.o misc.o sessPoll.o memPoll.o


run: build
	./$(OUT)

build: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ)
