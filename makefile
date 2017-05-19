CC=gcc

runClient: clientRun
	./clientRun

runServer: serverRun
	./serverRun

server: server.o
	$(CC) -o serverRun server.c -lm

client: client.o
	$(CC) -o clientRun client.c -lm

clean:
	rm clientRun serverRun client.o server.o

server.o: server.c
	gcc -c server.c -o server.o
