CC = g++
CFLAGS = -g

all:	P2

Helper.o:      Helper.hpp Helper.cpp
		$(CC) $(CFLAGS) -c Helper.cpp

Topology.o:      Topology.hpp Topology.cpp
		$(CC) $(CFLAGS) -c Topology.cpp

Node.o:      Node.hpp Node.cpp
		$(CC) $(CFLAGS) -c Node.cpp			

P2:    Helper.o Topology.o Node.o
		$(CC) $(CFLAGS) Helper.o Topology.o  Node.o main.cpp -o P2

clean:
		rm -f *.o *~ P2 core
