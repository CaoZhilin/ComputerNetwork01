#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <netdb.h>
#include <ctype.h>
#include "client.h"

int main(int argc, char *argv[]) {
    puts("Hello, this is an FTP client made by CaoZhilin.");
    start_ftp_client();
    return 0;
}

void usage_guide(char *ip, char *str_port){
    puts("userguide");
    char hostname[100] = {0};
    puts("If you didn`t input the ip address and port number, please follow the following instructions.");
    puts("Please enter the IP address or host name of the site.");
    printf("IP address(or host name): ");
    scanf("%s", hostname);
    
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    
    if ((host = gethostbyname(hostname)) == NULL) {
        herror("wrong with gethostbyname()");
        return;
    }
    
    addr_list = (struct in_addr **)host->h_addr_list;
    
    for (i = 0; addr_list[i] != NULL; i++) {
        strcpy(ip,inet_ntoa(*addr_list[i]));
    }
    
    puts("Please enter the port number(the deafult one is 21).");
    printf("Port number: ");
    scanf("%s", str_port);
}

int connect_to_server(char *ip, int port)
{
    struct sockaddr_in client_sock_addr;
    int res;
    client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (client_sock < 0)
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&client_sock_addr, 0, sizeof(struct sockaddr_in));
    
    client_sock_addr.sin_family = AF_INET;
    client_sock_addr.sin_port = htons(port);
    res = inet_pton(AF_INET, ip, &client_sock_addr.sin_addr);
    
    if (res == 0)
    {
        perror("The ip address is not valid!");
        close(client_sock);
        exit(EXIT_FAILURE);
    }
    
    if (connect(client_sock, (const struct sockaddr *)&client_sock_addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("connect failed");
        close(client_sock);
        exit(EXIT_FAILURE);
    }
    return 0;
}

void start_ftp_client()
{
    char ip[100] = {0};
    char str_port[100] = {0};
    usage_guide(ip, str_port);
    if(connect_to_server(ip, atol(str_port)) == 0) {
        int type = receive_server_msg();
        printf("%s",msg_buf);
        if(type != 0) {
            puts("Connected to the FTP server!");
            printf("%s",msg_buf);
        }
        else{
            puts("Receive failed!");
            return;
        }
    }
    
    puts("Now you can execute the following commands:");
    puts("LOGIN| PORT | PASV | RETR | STOR | SYST | TYPE | QUIT | ABOR | HELP");
    show_instructions();
    int login_flag = 1;
    while (1) {
        printf("-->");
        memset(cmd, '\0', sizeof(cmd));
        fgets(cmd, MAX_BUF, stdin);
        fflush(stdin);
        if(cmd[0] == '\n')continue;
        cmd[strlen(cmd) - 1] = '\0';
        
        if(strncmp(cmd, "LOGIN", 5) == 0)
        {
            login_flag = login();
        }
        else if(strncmp(cmd, "PORT", 4) == 0 && login_flag == 0)
        {
            cmd_port();
        }
        else if(strncmp(cmd, "PASV", 4) == 0 && login_flag == 0)
        {
            cmd_pasv();
        }
        else if(strncmp(cmd, "RETR ", 5) == 0 && login_flag == 0)
        {
            cmd_retr();
        }
        else if(strncmp(cmd, "STOR ", 5) == 0 && login_flag == 0)
        {
            cmd_stor();
        }
        else if(strncmp(cmd, "SYST", 4) == 0 && login_flag == 0)
        {
            cmd_syst();
        }
        else if(strncmp(cmd, "TYPE", 4) == 0 && login_flag == 0)
        {
            cmd_type();
        }
        else if(strncmp(cmd, "QUIT", 4) == 0 && login_flag == 0)
        {
            cmd_quit();
            return;
        }
        else if(strncmp(cmd, "ABOR", 4) == 0 && login_flag == 0)
        {
            cmd_abor();
            return;
        }
        else if(strncmp(cmd, "HELP", 4) == 0)
        {
            cmd_help();
        }
        else if(login_flag == 1){
            puts("Please log in first!");
        }
        else{
            puts("Your command is invalid!");
        }
    }
    
    
}

void cmd_port(){
    int flag = 0;
    char buf[100] = {0};
    int h1,h2,h3,h4,p1,p2;
    int msg_port = 0;
  
    char *p = strchr(cmd, ' ');
    if (p == NULL) {
        flag = 1;
    }
    else{
        while (*p == ' ') {
            p++;
        }
    }
    if (p == NULL || p == '\0') {
        flag = 1;
    }
    if (flag == 0) {
        sscanf(cmd, "PORT %d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1,&p2);
        msg_port = p1 *256 + p2;
        sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", h1, h2, h3, h4, p1, p2);
        send_server_msg(buf, strlen(buf));
        int type = receive_server_msg();
        printf("%s",msg_buf);
        if(type != 200)
        {
            printf("Can't connect with PORT mode.Try to change to PASV mode.\n");
            return;
        }
    }
    else{
        puts("Error:Please input PORT with ip and port.");
        return;
    }
    struct sockaddr_in local;
    int len = sizeof(local);
    char *addr;
    getsockname(client_sock, (struct sockaddr *)&local, (socklen_t *)&len);
    addr = inet_ntoa(local.sin_addr);
    memset(&data_transfer_addr, 0, sizeof(struct sockaddr_in));
    data_transfer_addr.sin_family = AF_INET;
    data_transfer_addr.sin_addr.s_addr= inet_addr(addr);
    data_transfer_addr.sin_port = htons(msg_port);
    
    mode = 1;
    puts("Changed transmision mode into PORT");
}
void cmd_pasv(){
    mode = 0;
    puts("Changed transmision mode into PASV");
}
//download
void cmd_retr(){
    int connect_data_sock = -1, i = 0;
    char filename[MAX_BUF] = {0};
    char local_file[MAX_BUF] = {0};
    char buf[MAX_BUF] = {0};
    char data[MAX_BUF] = {0};
    char cover_flag[3];
    struct stat file_info;
    int writeFD;
    
    get_cmd_filename(cmd, filename);
    strcpy(local_file,"tmpc");   //
    char *p = strrchr(filename,'/');
    if(p == NULL)
    {
        p = filename;
        strcat(local_file, "/");
    }
    strcat(local_file, p);
    if(!stat(local_file, &file_info))
    {
        printf("local file %s exists: %d bytes\n", local_file, (int)file_info.st_size);
        printf("Do you want to cover it? [y/n]");
        fgets(cover_flag, sizeof(cover_flag), stdin);
        fflush(stdin);
        if(cover_flag[0] == 'n')
        {
            printf("get file %s aborted.\n", filename);
            return;
        }
    }
    
    umask(0);
    writeFD = open(local_file, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU | S_IRWXO | S_IRWXG);
    
    if(writeFD < 0)
    {
        printf("create local file %s failed!\n", local_file);
        return;
    }
    data_sock = connect_server_data();
    if(data_sock < 0)
    {
        printf("socket error!\n");
        return;
    }
    
    memset(data, 0, sizeof(data));
    
    send_server_msg("TYPE I\r\n",strlen("TYPE I\r\n"));
    receive_server_msg();
    printf("%s",msg_buf);
    
    memset(buf, 0, sizeof(buf));
    strcpy(buf, "RETR ");
    strcat(buf, filename);
    strcat(buf,"\r\n");
    send_server_msg(buf,strlen(buf));
    receive_server_msg();
    printf("%s", msg_buf);
    
    if (mode == 0) //PASV mode
    {
        while(read(connect_data_sock, data, MAX_BUF)>0)
        {
            printf("loop \n");
            write(writeFD, data, strlen(data));
        }
        close(writeFD);
        close(data_sock);
        receive_server_msg();
        printf("%s", msg_buf); 
    }
    else{
        //check whether PORT is available
        while(i < 2)
        {
            connect_data_sock = accept(data_sock, NULL, NULL);
            if(connect_data_sock == -1)
            {
                i++;
                continue;
            }
            else break;
        }
        if(connect_data_sock == -1)
        {
            printf("Error in connect, can not use PORT mode\n");
            return;
        }
        while(read(connect_data_sock, data, MAX_BUF)>0)
        {
            write(writeFD, data, strlen(data));
        }
        close(writeFD);
        close(connect_data_sock);
        close(data_sock);
        receive_server_msg();
        printf("%s", msg_buf);
    }
}
//upload
void cmd_stor(){
    int connect_data_sock = -1, i = 0;
    char filename[MAX_BUF] = {0};
    char buf[MAX_BUF] = {0};
    char data[MAX_BUF] = {0};
    struct stat file_info;
    int readFD;
    
    get_cmd_filename(cmd, filename);
    if(stat(filename, &file_info) < 0)
    {
        printf("Local file %s doesn't exist!\n", filename);
        return;
    }
    readFD = open(filename, O_RDONLY);
    if(readFD < 0)
    {
        printf("Open local file %s error!\n", filename);
        return;
    }
    
    
    data_sock = connect_server_data();
    
    if(data_sock < 0)
    {
        printf("Create data sock error!\n");
        return;
    }
    
    send_server_msg("TYPE I\r\n",strlen("TYPE I\r\n"));
    receive_server_msg();
    printf("%s",msg_buf);

    memset(buf, 0, sizeof(buf));
    strcpy(buf, "STOR ");
    strcat(buf, filename);
    strcat(buf, "\r\n");
    send_server_msg(buf,strlen(buf));
    receive_server_msg();
    printf("%s",msg_buf);
    
    if (mode == 0) {   //PASV mode
        puts("check pasv");
        while(read(readFD, data, MAX_BUF) > 0)
        {
            printf("Uploading \n");
            write(data_sock, data, sizeof(data));
        }
        puts("Finished uploading");
        close(readFD);
        close(data_sock);
        receive_server_msg();
        printf("%s", msg_buf);
    }
    else{
        //check whether PORT is available
        puts("check port");
        while(i < 2)
        {
            connect_data_sock = accept(data_sock, NULL, NULL);
            if(connect_data_sock == -1)
            {
                i++;
                continue;
            }
            else break;
        }
        if(connect_data_sock == -1)
        {
            printf("Error in connect, can not use PORT mode\n");
            return;
        }
        while(read(readFD, data, MAX_BUF) > 0)
        {
            write(connect_data_sock, data, sizeof(data));
        }
        close(readFD);
        close(data_sock);
        close(connect_data_sock);
        receive_server_msg();
        printf("%s", msg_buf);
    }
}
int connect_server_data(){
    //struct sockaddr_in new_addr;
    if(mode == 1){ //PORT
        struct sockaddr_in local;
        int addr_len =  sizeof(struct sockaddr);
        int data_transfer_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (data_transfer_sock < 0){
		    perror("error create data socket!\n");
		    exit(EXIT_FAILURE);
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
            return -1;
        }
        while(bind(data_transfer_sock, (struct sockaddr*)&data_transfer_addr, sizeof(struct sockaddr_in)) < 0)  //if port is in use
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
            return -1;
        }
        
        if(getsockname(client_sock,(struct sockaddr*)&local,(socklen_t *)&addr_len) < 0){
            return -1;
		}
        long ip = inet_addr(inet_ntoa(local.sin_addr));                                          
		snprintf(addr_info_str, sizeof(addr_info_str), "PORT %ld,%ld,%ld,%ld,", ip&0xff, ip>>8&0xff, ip>>16&0xff, ip>>24&0xff);
		snprintf(port_str, sizeof(port_str), "%d,%d\r\n", port1, port2);
		strcat(addr_info_str, port_str);
		strcpy(pasv_msg, addr_info_str);
		send_server_msg(pasv_msg, strlen(pasv_msg));
		int type = receive_server_msg();
                printf("%s",msg_buf);
		if(type != 200)
		{
			printf("Can't connect in PORT mode!Try change to PASV mode.\n");
			return -1;
		}
		else
		{
			return data_transfer_sock;
		}
    }
    else{//PASV
        char msg_ip[50] = {0};
        int msg_port = 0;
        char mode_msg[] = "PASV\r\n";
        send_server_msg(mode_msg,strlen(mode_msg));
        puts("send pasv");
        //if (receive_server_msg() != 227) {
          //  puts("Something wrong with PASV mode, please change into PORT mode.");
            //return -1;
        //}
        receive_server_msg();
        printf("%s",msg_buf);
        get_ip_from_msg(msg_buf,msg_ip,&msg_port);
        
        memset(&data_transfer_addr, 0, sizeof(data_transfer_addr));
        data_transfer_addr.sin_family = AF_INET;
        data_transfer_addr.sin_port = htons(msg_port);
        if (inet_pton(AF_INET, msg_ip, &data_transfer_addr.sin_addr) == 0) {
            perror("tht PASV mode address is not valid.");
            return -1;
        }
        int new_data_sock = socket(AF_INET, SOCK_STREAM, 0);
        
        if (connect(new_data_sock,(struct sockaddr *)&data_transfer_addr,sizeof(struct sockaddr_in)) < 0)
        {
            printf("Can't connect to server %s, port %d\n", inet_ntoa(data_transfer_addr.sin_addr),ntohs(data_transfer_addr.sin_port));
            return -1;
        }
        return new_data_sock;
    }
}
void get_ip_from_msg(char *buffer,char *ip_address,int *port)
{
    char *p, *q, *r;
    int i;
    char port1[6] = {0};
    char port2[6] = {0};
    r = strchr(buffer, '(') + 1;
    p = r;
    for (i = 0; i < 4; i++)
    {
        p = strchr(p,',');
        *p = '.';
    }
    *p = '\0';
    strcpy(ip_address, r);
    p = p + 1;
    if((q = strchr(p, ',')) != NULL)
    {
        *q = '\0';
        strcpy(port1, p);
        q = q + 1;
        strcpy(port2, q);
    }
    *port = atoi(port1) * 256 + atoi(port2);
}
void cmd_syst(){
    send_server_msg("SYST\r\n",strlen("SYST\r\n"));
    receive_server_msg();
    printf("%s",msg_buf);
}
void cmd_type(){
    send_server_msg("TYPE I\r\n",strlen("TYPE I\r\n"));
    receive_server_msg();
    printf("%s",msg_buf);
}
//get retr comand filename(parameter)
void get_cmd_filename(const char *cmd, char *filename)
{
    char *p = strchr(cmd,' ');
    while(*p == ' ') p++;
    if(p != NULL && *p != '\0')
    {
        strcpy(filename, p);
    }
}

int login(){
    char user[MAX_BUF];
    char buf[MAX_BUF];
    printf("Username(Press Enter for anonymous): ");
    fgets(buf, MAX_BUF, stdin);
    if(buf[0]=='\n')
    {
        strncpy(user, "anonymous", 9);
    }
    else
    {
        strncpy(user, buf, strlen(buf)-1);
    }
    
    memset(buf, 0, sizeof(buf));
    strcpy(buf, "USER ");
    strcat(buf, user);
    strcat(buf,"\r\n");
    send_server_msg(buf,strlen(buf));
    
    int res = receive_server_msg();
    printf("%s",msg_buf);
    
    if(res == 331)
    {
        char pass[MAX_BUF];
        printf("Password(Press Enter for anonymous): ");
        memset(buf, 0, sizeof(buf));

        fgets(buf, sizeof(buf), stdin);
        if(buf[0]=='\n')
        {
            strncpy(pass, "anonymous", 9);
        }
        else
        {
            strncpy(pass, buf, strlen(buf)-1);
        }
        printf("\n");
        
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "PASS ");
        strcat(buf, pass);
        strcat(buf,"\r\n");
        send_server_msg(buf,strlen(buf));
        int ret = receive_server_msg();
        printf("%s",msg_buf);
        
        if(ret == 230)
        {
            return 0;
        }
        else
        {
            printf("Login failed!\n");
            return 1;
        }
    }
    return 0;
}

int send_server_msg(char* msg, int length)
{
    if(write(client_sock, msg, length)<0)
    {
        perror("Error send message to server\n");
        return 1;
    }
    return 0;
}

int receive_server_msg()
{
    int res = 0;
    memset(msg_buf, 0, sizeof(msg_buf));
    int ret = read(client_sock, msg_buf, MAX_BUF);
    if(ret > 0)
        res = atoi(msg_buf);
    else
        return 0;
    return res;
}

void show_instructions(){
    printf("\033[32mHELP\033[0m\t--print this command list\n");
    printf("\033[32mLOGIN\033[0m\t--login\n");
    printf("\033[32mPORT\033[0m\t--change the current mode into PORT\n");
    printf("\033[32mPASV\033[0m\t--change the current mode into PASV\n");
    printf("\033[32mRETR\033[0m\t--download file from the server\n");
    printf("\033[32mSTOR\033[0m\t--upload file to the server\n");
    printf("\033[32mSYST\033[0m\t--get the system info\n");
    printf("\033[32mTYPE\033[0m\t--set transmission type binary or ASCII\n");
    printf("\033[32mQUIT\033[0m\t--quit this ftp client program\n");
    printf("\033[32mABOR\033[0m\t--quit this ftp client program\n");
}
void cmd_quit(){
    send_server_msg("QUIT\r\n",strlen("QUIT\r\n"));
    receive_server_msg();
    printf("%s",msg_buf);
    close(client_sock);
    return;
}
void cmd_help(){
    show_instructions();
}
void cmd_abor(){
    send_server_msg("ABOR\r\n",strlen("ABOR\r\n"));
    receive_server_msg();
    printf("%s",msg_buf);
    close(client_sock);
    return;
}




