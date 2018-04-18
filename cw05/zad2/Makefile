CC = gcc
FIFO = fifo

all: compile test

compile:
	$(CC) master.c -o master
	$(CC) slave.c -o slave
	$(CC) main.c -o main

test:
	./main "$(FIFO)" 5 4

clean:
	rm $(FIFO)
	rm slave
	rm master
	rm main
