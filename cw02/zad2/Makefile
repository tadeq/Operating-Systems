CC = gcc
FLAGS = -std=c99 -Wall

all: compile tests
	
compile: 
	$(CC) $(FLAGS) main.c -o main

tests:
	./main "/opt" ">" "2018-03-03" nftw
	./main "/opt" "<" "2018-03-03" nftw
	./main "/opt" ">" "2018-03-03" 
	./main "/opt" "<" "2018-03-03" 
	./main "./" "=" "2018-03-22"
	./main "./" "=" "2018-03-22" nftw

clean:
	rm -f main
