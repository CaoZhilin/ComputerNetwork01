server:server.c
	gcc -Wall -o server server.c -lpthread
clean:
	rm server