CC = g++
CFLAGS = -c -Wall
all: cw

cw: main.o
	$(CC) main.o -o cw -lpng
main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp 

clean:
	@ rm -rf *.o