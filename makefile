#
# @brief Makefile for mrna
# @author Paul Florian Proell (1525669)
# 

CC			=	gcc
CFLAGS	=	-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g

all: mrna-server.o mrna-client.o
	$(CC) $(CFLAGS) -o mrna-server mrna-server.o
	$(CC) $(CFLAGS) -o mrna-client mrna-client.o

mrna-server.o: mrna-server.c
	$(CC) $(CFLAGS) -c mrna-server.c

mrna-client.o: mrna-client.c
	$(CC) $(CFLAGS) -c mrna-client.c

clean:
	rm -f mrna-server
	rm -f mrna-client
	rm -f -R *.o
