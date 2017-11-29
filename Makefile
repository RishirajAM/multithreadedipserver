CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: s c cgi

c: c.c csapp.o
	$(CC) $(CFLAGS) -o c c.c csapp.o q.o $(LIB)

s: s.c csapp.o q.o
	$(CC) $(CFLAGS) -o s s.c csapp.o q.o $(LIB)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

q.o: q.c
	$(CC) $(CFLAGS) -c q.c $(LIB)

cgi:
	(cd cgi-bin; make)

clean:
	rm -f *.o s c *~
	(cd cgi-bin; make clean)

