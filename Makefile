# Copyright 2023 <Niculici Mihai-Daniel>
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server
HASH=hash

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(HASH).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c -g

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c -g

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c -g

$(HASH).o: $(HASH).c $(HASH).h
	$(CC) $(CFLAGS) $^ -c -g

clean:
	rm -f *.o tema2 *.h.gch
