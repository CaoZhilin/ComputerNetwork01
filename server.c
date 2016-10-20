#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

#include "message.h"

#define BACKLOG 10

int send_client_message(int clientFD, char* message, int length);

int main(int argc, char **argv) {
	int listenfd, connfd;
	struct sockaddr_in addr;
	char sentence[8192];
	int p;
	int len;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Could not create socket");
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons( 21 );

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		puts("bind failed");
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	puts("bind done");

	if (listen(listenfd, BACKLOG) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	while (1) {
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}
		send_client_message(connfd,SeverReady220,strlen(SeverReady220));
		p = 0;
		while (1) {
			int n = read(connfd, sentence + p, 8191 - p);
			if (n < 0) {
				printf("Error read(): %s(%d)\n", strerror(errno), errno);
				close(connfd);
				continue;
			} else if (n == 0) {
				break;
			} else {
				p += n;
				if (sentence[p - 1] == '\n') {
					break;
				}
			}
		}

		sentence[p - 1] = '\0';
		len = p - 1;
		
		for (p = 0; p < len; p++) {
			sentence[p] = toupper(sentence[p]);
		}

		p = 0;
		while (p < len) {
			int n = write(connfd, sentence + p, len + 1 - p);
			if (n < 0) {
				printf("Error write(): %s(%d)\n", strerror(errno), errno);
				return 1;
	 		} else {
				p += n;
			}			
		}

		close(connfd);
	}

	close(listenfd);
	return 0;
}

int send_client_message(int clientFD, char* message, int length){
    if(write(clientFD, message, length) < 0){
        perror("Could not sent message to client, there is something wrong!")
        return 1;
    }
    return 0;
}

