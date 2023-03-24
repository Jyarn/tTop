CC=gcc
CFLAGS=-g -Wall
OUT=ttop.out
OBJ=main.o cpuPoll.o misc.o sessPoll.o memPoll.o IPC.o
RARGS=--sequential --graphics --samples=20 --tdelay=2

run: build
	./$(OUT) $(RARGS)

build: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(OUT)

testIPC: ipcTest.o IPC.o
	$(CC) $(CFLAGS) ipcTest.o IPC.o -o testIPC.out
	./testIPC.out

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ) ipcTest.*
