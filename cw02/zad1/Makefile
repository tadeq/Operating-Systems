CC = gcc
FLAGS = -std=c99 -Wall

all: compile tests
	
compile: 
	$(CC) $(FLAGS) main.c -o main

tests:
	./main generate dane1.txt 10000 4
	./main copy dane1.txt kopia1.txt 10000 4 sys
	./main copy dane1.txt kopia2.txt 10000 4 lib
	./main sort kopia1.txt 10000 4 sys
	./main sort kopia2.txt 10000 4 lib
	./main generate dane2.txt 5000 4
	./main copy dane2.txt kopia3.txt 5000 4 sys
	./main copy dane2.txt kopia4.txt 5000 4 lib
	./main sort kopia3.txt 5000 4 sys
	./main sort kopia4.txt 5000 4 lib
	./main generate dane3.txt 5000 512
	./main copy dane3.txt kopia5.txt 5000 512 sys
	./main copy dane3.txt kopia6.txt 5000 512 lib
	./main sort kopia5.txt 5000 512 sys
	./main sort kopia6.txt 5000 512 lib
	./main generate dane4.txt 3000 512
	./main copy dane4.txt kopia7.txt 3000 512 sys
	./main copy dane4.txt kopia8.txt 3000 512 lib
	./main sort kopia7.txt 3000 512 sys
	./main sort kopia8.txt 3000 512 lib
	./main generate dane5.txt 3000 4096
	./main copy dane5.txt kopia9.txt 3000 4096 sys
	./main copy dane5.txt kopia10.txt 3000 4096 lib
	./main sort kopia9.txt 3000 4096 sys
	./main sort kopia10.txt 3000 4096 lib
	./main generate dane6.txt 2000 4096
	./main copy dane6.txt kopia11.txt 2000 4096 sys
	./main copy dane6.txt kopia12.txt 2000 4096 lib
	./main sort kopia11.txt 2000 4096 sys
	./main sort kopia12.txt 2000 4096 lib
	./main generate dane7.txt 3000 8192
	./main copy dane7.txt kopia13.txt 3000 8192 sys
	./main copy dane7.txt kopia14.txt 3000 8192 lib
	./main sort kopia13.txt 3000 8192 sys
	./main sort kopia14.txt 3000 8192 lib
	./main generate dane8.txt 2000 8192
	./main copy dane8.txt kopia15.txt 2000 8192 sys
	./main copy dane8.txt kopia16.txt 2000 8192 lib
	./main sort kopia15.txt 2000 8192 sys
	./main sort kopia16.txt 2000 8192 lib

clean:
	rm -f main
	rm -f *.o
	rm -f *dane*
	rm -f *kopia*
