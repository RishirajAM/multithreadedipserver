CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: s c

c: c.c csapp.o
	$(CC) $(CFLAGS) -o c c.c csapp.o $(LIB)

s: s.c csapp.o
	$(CC) $(CFLAGS) -o s s.c csapp.o $(LIB)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

clean:
	rm -f *.o s c *~

