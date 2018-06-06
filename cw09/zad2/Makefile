CC = gcc
FLAGS = -lpthread

all: compile test

compile:
	$(CC) main.c -o main $(FLAGS)

test:
	./main config.txt

clean:
	rm main
