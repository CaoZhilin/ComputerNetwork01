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
    
    int serverFD,clientFD;
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
		if ((clientFD = accept(serverFD,(struct sockaddr *)&client,&sockin_size) )== -1) {
            puts("Accept failed!");
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}
        puts("Connection accepted");
        send_client_message(clientFD,msgServerReady220,strlen(msgServerReady220));
        
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
        strcpy(default_user[i], username);
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
    int login_flag = 0;//0-not logged in; 1-anonymous user; 2-default user
    //wait for the user to requeset "USER" command
    while(1)
    {
        if(receive_client_message(clientFD) && strncmp("USER ", msg_buf, 5)==0)
        {
            break;
        }
        else
        {
            send_client_message(clientFD, msgNotImplemented202, strlen(msgNotImplemented202));
        }
    }
    //get username
    int length = strlen(msg_buf);
    char username[MAX_BUF]={0};
    char password[MAX_BUF]={0};
    int i;
    for(i=5; i<length; i++)
    {
        username[i-5] = msg_buf[i];
    }
    username[i-6] = '\0';
    
    if(strncmp(username, "anonymous",9)==0)
    {
        send_client_message(clientFD,msgInputPassword331,strlen(msgInputPassword331));
        while(1)
        {
            if(receive_client_msg(clientFD) && strncmp("PASS ", msg_buf, 5)==0)
            {
                break;
            }
            else
            {
                send_client_message(clientFD,msgInputPassword331, strlen(msgInputPassword331));
            }
        }
        length = strlen(msg_buf);
        memset(password, 0, sizeof(password));
        for(i = 5; i < length; i++)
        {
            password[i-5] = msg_buf[i];
        }
        password[i-6] = '\0';
        printf("Client %d logged in, username:%s password:%s\n", getpid(), username, password);
        send_client_message(clientFD, msgWelcome230, strlen(msgWelcome230));
        login_flag = 1;
    }
    else
    {
        send_client_message(clientFD,serverMsg331,strlen(serverMsg331));
        while(1)
        {
            if(receive_client_msg(clientFD) && strncmp("PASS ", msg_buf, 5)==0)
            {
                break;
            }
            else
            {
                send_client_message(clientFD, msgInputPassword331, strlen(msgInputPassword331));
            }
        }
        
        int flag = 0;            //allow to login or not
        length = strlen(msg_buf);
        
        for(i = 5; i < length; i++)
        {
            password[i-5] = msg_buf[i];
        }
        password[i-6] = '\0';
        
        for(i = 0; i < default_user_amount; i++){
            if (strcmp(default_user[i],username)==0 && strcmp(default_pwd[i],password)==0)
            {
                flag = 1;
                break;
            }
        }
        
        if(flag == 1)
        {
            printf("client %d logged in, username:%s password:%s\n",getpid(),username,password);
            send_client_message(clientFD, msgWelcome230, strlen(msgWelcome230));
            login_flag = 2;
        }
        else
        {
            send_client_message(clientFD, msgNotLoggedIn530, strlen(msgNotLoggedIn530));
            login_flag = 0;
        }
    }
    //process other request
    while (receive_client_message(clientFD)) {
        if(strncmp("QUIT", msg_buf, 4)==0 ||strncmp("ABOR", msg_buf, 4)==0 )
        {
            char buf[MAX_BUF] = {0};
            char buf2[MAX_BUF] = {0};
            if(numOfDownloadFiles != 0)
            {
                snprintf(buf, MAX_BUF, "221-You have download %ld bytes in %ld files,\r\n", bytesOfDownload, numOfDownloadFiles);
            }
            if(numOfUploadFiles != 0)
            {
                snprintf(buf2, MAX_BUF, "221-upload %ld bytes in %ld files\r\n", bytesOfUpload, numOfUploadFiles);
            }
            strcat(buf, buf2);
            strcat(buf,serverMsg221);
            send_client_message(clientFD, buf, strlen(buf));
            close(clientFD);
            close(serverFD);
            close(connect_data_sock);
            close(ftp_data_sock);
            exit(0);
        }
        else if(login_flag > 0 && strncmp("SYST", msg_buf, 4) == 0)
        {
            send_client_message(clientFD, serverMsg215, strlen(serverMsg215));
        }
        else if(login_flag > 0 &&strncmp("PASV", msg_buf, 4)== 0)
        {
            Pasv(clientFD, client);
        }
        else if(login_flag > 0 && strncmp("PORT ", msg_buf, 5)== 0)
        {
            Port(clientFD);
        }
        else if(login_flag > 0 && strncmp("TYPE", msg_buf, 4)== 0)
        {
            if(strncmp("TYPE I", msg_buf,6) == 0)
            {
                send_client_message(clientFD, TYPE_I_SUCCESS, strlen(TYPE_I_SUCCESS));
            }
            else
            {
                send_client_message(clientFD, msgNotImplemented202, strlen(msgNotImplemented202));
            }
        }
        else if(login_flag > 0 && strncmp("RETR ", msg_buf, 5)== 0)
        {
            Retr(clientFD);
        }
        else if(login_flag > 0 && strncmp("STOR ", msg_buf, 5)== 0)
        {
            Stor(clientFD);
        }
        else if(login_flag > 0 && strncmp("MKD ", msg_buf, 4) == 0)
        {
            char pwd_info[MAX_BUF], dir[MAX_BUF], path[MAX_BUF];
            int length = strlen(msg_buf);
            int i;
            for(i = 4; i < length; i++)
            {
                dir[i-4] = msg_buf[i];
            }
            dir[i-5] = '\0';
            strcpy(path, pwd);
            strcat(path, dir);
            if(path[strlen(path)-1] != '/') strcat(path, "/");
            mkdir_r(path);
            snprintf(pwd_info, MAX_BUF, "257 \"%s\" directory created.\r\n", path);
            send_client_message(clientFD, pwd_info, strlen(pwd_info));
        }
        else
        {
            send_client_message(clientFD, msgNotImplemented202, strlen(msgNotImplemented202));
        }
    }
}

int receive_client_message(int clientFD)
{
    memset(msg_buf, 0, sizeof(msg_buf));
    int ret = read(clientFD, msg_buf, MAX_BUF);
    struct sockaddr_in addr;
    int len = sizeof(addr);
    if (ret == 0)   //client closed
    {
        puts("Client closed");
        return 0;
    }
    else if (ret == -1)
    {
        perror("Read Message from client failed");
        return 0;
    }
    getpeername(clientFD, (struct sockaddr *)&addr, &len);
    printf("Client %s send a message:%s",inet_ntoa(addr.sin_addr),msg_buf);
    return 1;
}

void Pasv(int clientFD, struct sockaddr_in client){
    int data_transfer_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (data_transfer_sock < 0)
    {
        printf("error create data socket!\n");
        return ;
    }
    
    srand((int)time(0));
    int port = rand()  + 20000;
    
    char pasv_msg[MAX_BUF];
    char port_str[8];
    char addr_info_str[30];
    
    memset(&data_transfer_addr, 0, sizeof(struct sockaddr_in));
    data_transfer_addr.sin_family = AF_INET;
    data_transfer_addr.sin_addr.s_addr= INADDR_ANY;
    data_transfer_addr.sin_port = htons(port);
    int port1 = ntohs(data_transfer_addr.sin_port) / 256;
    int port2 = ntohs(data_transfer_addr.sin_port) % 256;
    
    int on = 1;
    if(setsockopt(data_transfer_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0)
    {
        perror("error setsockopt!\n");
        close(data_transfer_sock);
        return;
    }
    while (bind(data_transfer_sock, (struct sockaddr*)&data_transfer_addr, sizeof(struct sockaddr_in)) < 0)  //if port is in use
    {
        port = rand()  + 20000;
        data_transfer_addr.sin_port = htons(port);
        port1 = ntohs(data_transfer_addr.sin_port) / 256;
        port2 = ntohs(data_transfer_addr.sin_port) % 256;
    }
    
    if(listen(data_transfer_sock, 10) < 0)
    {
        perror("error listen failed");
        close(data_transfer_sock);
        return;
    }
    
    long ip = inet_addr(inet_ntoa(client.sin_addr));  //get server's ip
    int h1,h2,h3,h4,p1,p2;
    sscanf(addr, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
    char buf[100];
    sprintf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\n", h1, h2, h3, h4, p1, p2);
    char *message = buf;
    send_client_message(clientFD,message,strlne(message));
    
    connect_mode = 0;
    
    if(ftp_data_sock > 0)
    {
        close(connect_data_sock);
        close(ftp_data_sock);
    }
    ftp_data_sock = data_transfer_sock;
}
void Port(int clientFD){
    char ip_address[50] = {0}
    int port = 0;
    getIPInfo(msg_buf,ip_address,&port_num);
    memset(&data_transfer_addr, 0, sizeof(struct sockaddr_in));
    data_transfer_addr.sin_family = AF_INET;
    data_transfer_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip_address, &data_transfer_addr.sin_addr) == 0)
    {
        printf("second parameter does not contain valid ipaddress");
        return;
    }
    //change to port
    connect_mode = 1;
    if(ftp_data_sock > 0)
    {
        close(ftp_data_sock);
        close(connect_data_sock);
        ftp_data_sock = -1;
        connect_data_sock = -1;
    }
    send_client_message(clientFD, msgPortSuc200, strlen(msgPortSuc200));
}
void get_file_path(char *path,char *filename){
    int i = 0;
    int length = strlen(msg_buf);
   
    
    for(i = 5; i < length; i++)
    {
        filename[i-5] = msg_buf[i];
    }
    filename[i-6] = '\0';
    
    strcpy(path, pwd);
    strcat(path, filename);
}
void Retr(int clientFD){
 
    long file_size = 0;
    char size[MAX_BUF];
    char msg[MAX_BUF];
    char path[MAX_BUF];
    char filename[MAX_BUF] = {0};
    struct stat file_info;
    
    get_file_path(path,filename);
    
    // get file status
    if(!stat(path, &file_info))
    {
        file_size = (int)file_info.st_size;
    }
    snprintf(size, sizeof(size), "%s(%ld bytes).\r\n",filename,file_size);
    strcpy(msg, msgRetr150);
    strcat(msg, size);
    send_client_message(clientFD, msg, strlen(msg));  //send message to client
    
    
    //send file
    int readFD = open(path, O_RDONLY);
    if(readFD < 0)
    {
        send_client_message(clientFD, msgReadFailed451, strlen(msgReadFailed451));
        close(connect_data_sock);
        close(ftp_data_sock);
        return;
    }
    char data[MAX_BUF];
    memset(data, 0, sizeof(data));
    
    if(connect_mode == 0)   //PASV mode
    {
        if(ftp_data_sock < 0)
        {
            send_client_message(clientFD, msgOpenConnectionF425, strlen(msgOpenConnectionF425));
            close(connect_data_sock);
            close(ftp_data_sock);
            close(readFD);
            return;
        }
        //accpet
        connect_data_sock = accept(ftp_data_sock, NULL, NULL);
        if(connect_data_sock < 0)
        {
            send_client_message(clientFD, msgOpenConnectionF425, strlen(msgOpenConnectionF425));
            close(connect_data_sock);
            close(ftp_data_sock);
            close(readFD);
            return;
        }
        //read and send
        while (read(readFD, data, MAX_BUF) > 0)
        {
            if (send(connect_data_sock, data, strlen(data), 0) < 0)
            {
                send_client_message(clientFD, serverMsg426, strlen(serverMsg426));
                close(connect_data_sock);
                close(ftp_data_sock);
                close(readFD);
                return;
            }
            memset(data, 0, sizeof(data));
        }
    }
    else       //PORT mode
    {
        connect_data_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(connect_data_sock, (const struct sockaddr *)&data_transfer_addr, sizeof(struct sockaddr_in)) == -1)
        {
            send_client_message(clientFD, msgOpenConnectionF425, strlen(msgOpenConnectionF425));
            close(connect_data_sock);
            close(ftp_data_sock);
            close(readFD);
            return;
        }
        while (read(readFD, data, MAX_BUF) > 0)
        {
            if (send(connect_data_sock, data, strlen(data), 0) < 0)
            {
                send_client_message(clientFD, serverMsg426, strlen(serverMsg426));
                close(connect_data_sock);
                close(ftp_data_sock);
                close(readFD);
                return;
            }
            memset(data, 0, sizeof(data));  
        }
    }
    
    bytesOfDownload += file_size;
    numOfDownloadFiles ++;
    close(connect_data_sock);
    close(ftp_data_sock);
    close(readFD);
    ftp_data_sock = -1;
    connect_data_sock = -1;
    send_client_message(clientFD, msgTranComp226, strlen(msgTranComp226));
    
}
void Stor(int clientFD){
    char msg[MAX_BUF];
    char path[MAX_BUF];
    char filename[MAX_BUF] = {0};
    long filesize = 0;
    struct stat file_info;
    
    get_file_path(path,filename);
    
    strcpy(msg, msgStor150);
    strcat(msg, filename);
    strcat(msg, "\r\n");
    send_client_message(clientFD, msg, strlen(msg));  //send message to client

    //create file
    mkdir_r(path);
    umask(0);
    int writeFD = open(path, (O_RDWR | O_CREAT | O_TRUNC), S_IRWXU | S_IRWXO | S_IRWXG);
    if(writeFD < 0)
    {
        send_client_message(clientFD,  msgWriteFailed451, strlen(msgWriteFailed451));
        close(connect_data_sock);
        close(ftp_data_sock);
        return;
    }
    char data[MAX_BUF];
    memset(data, 0, sizeof(data));

    //transfer file
    if(connect_mode == 0)   //PASV mode
    {
        if(ftp_data_sock < 0)
        {
            send_client_message(clientFD, msgOpenConnectionF425, strlen(msgOpenConnectionF425));
            close(connect_data_sock);
            close(ftp_data_sock);
            close(writeFD);
            return;
        }
        connect_data_sock = accept(ftp_data_sock, NULL, NULL);
        if(connect_data_sock < 0)
        {
            send_client_message(clientFD, msgOpenConnectionF425, strlen(msgOpenConnectionF425));
            close(connect_data_sock);
            close(ftp_data_sock);
            close(writeFD);
            return;
        }
        while (read(connect_data_sock, data, MAX_BUF) > 0)
        {
            if (write(writeFD, data, strlen(data)) < 0)
            {
                send_client_message(clientFD, serverMsg426, strlen(serverMsg426));
                close(connect_data_sock);
                close(ftp_data_sock);
                close(writeFD);
                return;
            }
            memset(data, 0, sizeof(data));
        }
    }
    else //PORT mode
    {
        connect_data_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == connect(connect_data_sock, (const struct sockaddr *)&data_transfer_addr, sizeof(struct sockaddr_in)))
        {
            send_client_message(clientFD, msgOpenConnectionF425, strlen(msgOpenConnectionF425));
            close(connect_data_sock);
            close(ftp_data_sock);
            close(writeFD);
            return;
        }
        
        while (read(connect_data_sock, data, MAX_BUF) > 0)
        {
            if (write(writeFD, data, strlen(data)) < 0)
            {
                send_client_message(clientFD, serverMsg426, strlen(serverMsg426));
                close(connect_data_sock);
                close(ftp_data_sock);
                close(writeFD);
                return;
            }
            memset(data, 0, sizeof(data));
        }
    }
    close(connect_data_sock);
    close(ftp_data_sock);
    close(writeFD);
    ftp_data_sock = -1;
    connect_data_sock = -1;
    if(!stat(path, &file_info))
    {
        filesize = (int)file_info.st_size;
    }
    numOfUploadFiles ++;
    bytesOfUpload += filesize;
    send_client_message(clientFD, serverMsg226, strlen(serverMsg226));
}

int mkdir_r(const char *path)
{
    if (path == NULL)
    {
        return -1;
    }
    
    char *temp = malloc(MAX_BUF*sizeof(char));
    
    int i;
    for(i = 0; i<strlen(path); i++)
    {
        *(temp+i) = *(path+i);
    }
    
    char *pos = temp;
    
    
    if (strncmp(temp, "/", 1) == 0)
    {
        pos += 1;
    }
    else if (strncmp(temp, "./", 2) == 0)
    {
        pos += 2;
    }
    
    for ( ; *pos != '\0'; pos++)
    {
        if (*pos == '/')
        {
            *pos = '\0';
            umask(0);
            mkdir(temp, S_IRWXU | S_IRWXO | S_IRWXG);
            *pos = '/';
        }
    }
    free(temp);
    return 0;
}
