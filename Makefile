server:server.c message.h
	gcc -Wall -o server server.c -lpthread
client:client.c client.h
	gcc -Wall -o client client.c -lpthread
clean:
	rm server client
cleanc:
	rm client
cleans:
	rm server
