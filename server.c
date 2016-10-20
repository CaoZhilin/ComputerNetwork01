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

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#include "message.h"

#define BACKLOG 10


int main(int argc, char *argv[]) {
    load_in_default_users();
    process_terminal_arguments(argc, argv);
    
    int serverFD;
    struct sockaddr_in server,client;
    
    socklen_t sockin_size = sizeof(struct sockaddr_in);
    
    if ((serverFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Could not create socket");
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    
    int on = 1;
    if(setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int))==-1)
    {
        puts("setsockopt");
        close(serverFD);
        return 1;
    }
    
    if (bind(serverFD, (struct sockaddr*)&server, sizeof(server)) == -1) {
        puts("Bind failed!");
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
    puts("bind done");
    
    if (listen(serverFD, BACKLOG) == -1) {
        puts("Listen failed!");
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
    puts("Waiting for incoming connections...");
    
	while (1) {
		if ((clientFD = accept(serverFD,(struct sockaddr *)&client,&sockin_size) == -1) {
            puts("Accept failed!");
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}
        puts("Connection accepted")
		send_client_message(clientFD,msgSeverReady220,strlen(msgSeverReady220));
        
        int pid = fork();
        if (pid == -1)
        {
            printf("Error fork(): %s(%d)\n", strerror(errno), errno);
        }
        else if (pid == 0)
        {
            puts("Fork failed!");
            puts("Handler assigned");
            client_request_handler(clientFD, client);
        }
        close(clientFD);
	}
	close(serverFD);
    close(ftp_data_sock);
	return 0;
}

void load_in_default_users()
{
    FILE *fp;
    char username[MAX_BUF],password[MAX_BUF];
    if((fp = fopen("user.txt","a+")) == NULL)
    {
        puts("Error in loading default user\n");
        return;
    }
    fscanf(fp, "%d", &default_user_amount);
    int i;
    for(i=0; i<default_user_amount; i++)
    {
        fscanf(fp, "%s %s",username, password);
        strcpy(default_user[i], user);
        strcpy(default_pwd[i], password);
    }
    fclose(fp);
}

void process_terminal_arguments(int argc, char* argv[]){
    port = SERVER_PORT;
    strcpy(ftp_path,"/tmp/");
    int i;
    for(i = 0; i < argc; i++)        //check terminal arguments
    {
        if(strncmp(argv[i],"-port",5) == 0 && i+1 < argc)
        {
            port = atoi(argv[i+1]);
            i++;
        }
        else if(strncmp(argv[i],"-root",5) == 0 && i+1 < argc)
        {
            strcpy(ftp_path,argv[i+1]);
            i++;
        }
    }
    if(ftp_path[strlen(ftp_path)-1] != '/')
        strcat(ftp_path,"/");
    strcpy(pwd, ftp_path);
}

int send_client_message(int clientFD, char* message, int length){
    if(write(clientFD, message, length) < 0){
        perror("Could not sent message to client, there is something wrong!");
        return 1;
    }
    return 0;
}

void client_request_handler(int clientFD, struct sockaddr_in client){
    
}