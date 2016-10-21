server:server.c message.h
	gcc -Wall -o server server.c -lpthread
client:client.c
	gcc -Wall -o client client.c -lpthread
clean:
	rm server
