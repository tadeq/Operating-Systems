CC = gcc
FLAGS = -lm -lpthread

all: compile test

compile:
	$(CC) main.c -o main $(FLAGS)

test:
	./main 1 mona_lisa.ascii.pgm filter3.txt out_mona_lisa.pgm
	./main 1 mona_lisa.ascii.pgm filter3.txt out_mona_lisa.pgm
	./main 1 mona_lisa.ascii.pgm filter3.txt out_mona_lisa.pgm
	./main 1 mona_lisa.ascii.pgm filter3.txt out_mona_lisa.pgm
	./main 2 mona_lisa.ascii.pgm filter23.txt out_mona_lisa.pgm
	./main 2 mona_lisa.ascii.pgm filter23.txt out_mona_lisa.pgm
	./main 2 mona_lisa.ascii.pgm filter23.txt out_mona_lisa.pgm
	./main 2 mona_lisa.ascii.pgm filter23.txt out_mona_lisa.pgm
	./main 4 mona_lisa.ascii.pgm filter44.txt out_mona_lisa.pgm
	./main 4 mona_lisa.ascii.pgm filter44.txt out_mona_lisa.pgm
	./main 4 mona_lisa.ascii.pgm filter44.txt out_mona_lisa.pgm
	./main 4 mona_lisa.ascii.pgm filter44.txt out_mona_lisa.pgm
	./main 8 mona_lisa.ascii.pgm filter65.txt out_mona_lisa.pgm
	./main 8 mona_lisa.ascii.pgm filter65.txt out_mona_lisa.pgm
	./main 8 mona_lisa.ascii.pgm filter65.txt out_mona_lisa.pgm
	./main 8 mona_lisa.ascii.pgm filter65.txt out_mona_lisa.pgm

clean:
	rm main

