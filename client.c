#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>

int main(int argc, char **argv) {
    char ip[100];
    puts("This is an FTP client.");
    char hostname[100];
    printf("Please enter the address of the site first\n");
    printf("Address: ");
    scanf("%s", hostname);
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
    
    if ((he = gethostbyname(hostname)) == NULL){
        herror("gethostbyname");
        return 1;
    }
    
    addr_list = (struct in_addr **)he->h_addr_list;
    
    for (i = 0; addr_list[i] != NULL; i++){
        strcpy(ip, inet_ntoa(*addr_list[i]));
    }
    char server_reply[2000];
    int socket_desc, size;
    struct sockaddr_in server;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socket_desc == -1){
        printf("Could not create socket");
        return 1;
    }
    
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(21);
    
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0){
        puts("connect error");
        return 1;
    }
    if((size = read(socket_desc, server_reply, 2000)) < 0){
        puts("Receive failed");
        return 1;
    }
    puts("Connected to the FTP server!");
    server_reply[size] = '\0';
    printf("%s", server_reply);
    
	close(socket_desc);

	return 0;
}
