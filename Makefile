server:server.c message.h
	gcc -Wall -o server server.c -lpthread
clean:
	rm server
