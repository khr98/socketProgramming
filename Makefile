all: socket
	gcc -o socket socket.o

socket: socket.o
	gcc -c socket.c

clean:
	rm -rf *.o
	rm socket
