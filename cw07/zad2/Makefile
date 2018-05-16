CC = gcc
FLAGS = -Wall -o 

all:
	$(CC) $(FLAGS) barber barber.c -lrt -pthread
	$(CC) $(FLAGS) client client.c -lrt -pthread

clean:
	rm barber
	rm client
