CC=gcc
CFLAGS=-g -Wall
OUT=ttop.out
OBJ=main.o


run: build
	./$(OUT)

build: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@