#
# @brief Makefile for mrna
# @author Paul Florian Proell (1525669)
# 

CC			=	gcc
CFLAGS	=	-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g

all: mrna-server.o mrna-client.o
	$(CC) $(CFLAGS) -o mrna-server mrna-server.o -lrt -lpthread
	$(CC) $(CFLAGS) -o mrna-client mrna-client.o -lrt -lpthread

mrna-server.o: mrna-server.c
	$(CC) $(CFLAGS) -c mrna-server.c -lrt

mrna-client.o: mrna-client.c
	$(CC) $(CFLAGS) -c mrna-client.c -lrt

clean:
	rm -f mrna-server
	rm -f mrna-client
	rm -f -R *.o
