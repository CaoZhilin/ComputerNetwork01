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
#include <termios.h>
#include <memory.h>
#include "client.h"

int main(int argc, char *argv[]) {
    puts("Hello, this is an FTP client made by CaoZhilin.");
    if (argc == 1) {
        connect_to_server(argv[1], DEFAULT_SERVER_PORT);
    }
    else if(argc == 2){
        connect_to_server(argv[1], atol(argv[2]));
    }
    else{
        start_ftp_client();
        //connect_to_server(ip, atol(str_port));
    }
    return 0;
}

void usage_guide(char *ip, char *str_port){
    char hostname[100] = {0}
    //char str_port[100] = {0}
    puts("If you didn`t input the ip address and port number, please follow the following instructions.");
    puts("Please enter the IP address or host name of the site.");
    printf("IP address(or host name): ");
    scanf("%s", hostname);
    
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    
    if ((host = gethostbyname(hostname)) == NULL) {
        herror("wrong with gethostbyname()");
        return 1;
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
    
    if (0 == res)
    {
        perror("The ip address is not valid!");
        close(client_sock);
        exit(EXIT_FAILURE);
    }
    
    if (-1 == connect(client_sock, (const struct sockaddr *)&client_sock_addr, sizeof(struct sockaddr_in)))
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
    if (connect_to_server(ip, atol(str_port)) == 0) {
        if (receive_server_msg() != 0) {
            puts("Connected to the FTP server!");
            printf("%s",msg_buf);
        }
        else{
            puts("Receive failed!");
            return 1;
        }
    }
    
    puts("Now you can execute the following commands:");
    puts("login| port | pasv | retr | stor | syst | type | quit | abor | help");
    show_instructions();
    int login_flag = 1;
    while (1) {
        printf("ftp>");
        memset(cmd, '\0', sizeof(cmd));
        fgets(cmd, MAX_BUF, stdin);
        fflush(stdin);
        if(cmd[0] == '\n')continue;
        cmd[strlen(cmd) - 1] = '\0';
        
        if(strncmp(cmd, "login ", 6) == 0)
        {
            login_flag = login();
        }
        else if(strncmp(cmd, "port ", 5) == 0)
        {
            cmd_port();
        }
        else if(strncmp(cmd, "pasv ", 5) == 0)
        {
            cmd_pasv();
        }
        else if(strncmp(cmd, "retr ", 5) == 0)
        {
            cmd_retr();
        }
        else if(strncmp(cmd, "stor ", 5) == 0)
        {
            cmd_stor();
        }
        else if(strncmp(cmd, "syst ", 5) == 0)
        {
            cmd_syst();
        }
        else if(strncmp(cmd, "type ", 5) == 0)
        {
            cmd_type();
        }
        else if(strncmp(cmd, "quit ", 5) == 0)
        {
            cmd_quit();
        }
        else if(strncmp(cmd, "abor ", 5) == 0)
        {
            cmd_abor();
        }
        else if(strncmp(cmd, "help ", 5) == 0)
        {
            cmd_help();
        }
    }
    
    
}

void cmd_port(){
    int falg = 0;
    char buf[100] = {0};
    int h1,h2,h3,h4,p1,p2;
    int size = 0;
    int msg_port = 0;
    char buf[100], reply[1000];
    //scanf("%d", &port);
    p1 = port / 256;
    p2 = port % 256;
    
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
        if(receive_server_msg() != 200)
        {
            printf("Can not use PORT mode!Please use \"mode\" change to PASV mode.\n");
            return -1;
        }
        else
        {
            return data_transfer_sock;
        }
    }
    else{
        puts("Error:Please inuput PORT with ip and port.");
    }
    struct sockaddr_in local;
    int len = sizeof(local);
    getsockname(sock, (struct sockaddr *)&local, (socklen_t *)&len);
    addr = inet_ntoa(local.sin_addr);
    memset(&data_transfer_addr, 0, sizeof(struct sockaddr_in));
    char *addr;
    data_transfer_addr.sin_family = AF_INET;
    data_transfer_addr.sin_addr.s_addr= inet_addr(addr);
    data_transfer_addr.sin_port = htons(port);
    
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
    strcpy(local_file,"/tmp");   //
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
    
    memset(buf, 0, sizeof(buf));
    strcpy(buf, "STOR ");
    strcat(buf, filename);
    strcat(buf, "\r\n");
    send_server_msg(buf,strlen(buf));
    receive_server_msg();
    printf("%s",msg_buf);
    data_sock = connect_server_data();
    if(data_sock < 0)
    {
        printf("Creat data sock error!\n");
        return;
    }
    
    send_server_msg("TYPE I\r\n",strlen("TYPE I\r\n"));
    receive_server_msg();
    printf("%s",msg_buf);
    
    if (mode == 0) {   //PASV mode
        while(read(readFD, data, MAX_BUF) > 0)
        {
            printf("Uploading \n");
            write(data_sock, data, sizeof(data));
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
void connect_server_data(){
    //struct sockaddr_in new_addr;
    if(mode == 1){ //PORT
        struct sockaddr_in local;
        int addr_len =  sizeof(struct sockaddr);
        int data_transfer_sock = socket(AF_INET, SOCK_STREAM, 0);
        
        int on = 1;
        if(setsockopt(data_transfer_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0)
        {
            perror("error setsockopt!\n");
            close(data_transfer_sock);
            exit(EXIT_FAILURE);
        }
        if(bind(data_transfer_sock, (struct sockaddr*)&data_transfer_addr, sizeof(struct sockaddr_in)) < 0)  //if port is in use
        {
            return -1;
        }
        
        if(listen(data_transfer_sock, 10) < 0)
        {
            perror("error listen failed");
            close(data_transfer_sock);
            exit(EXIT_FAILURE);
        }
        
        if(getsockname(client_sock,(struct sockaddr*)&local,(socklen_t *)&addr_len) < 0)
            return -1;

    }
    else{//PASV
        char msg_ip[50] = {0};
        int msg_port = 0;
        char mode_msg[] = "PASV\r\n";
        send_server_msg(mode_msg,strlen(mode_msg));
        if (receive_server_msg() != 227) {
            puts("Something wrong with PASV mode, please change into PORT mode.");
            return -1;
        }
        get_ip_from_msg(msg_buf,msg_ip,&msg_port);
        
        memset(&data_transfer_addr, 0, sizeof(sockaddr_in));
        data_transfer_addr.sin_family = AF_INET;
        data_transfer_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, msg_ip, &data_transfer_addr.sin_addr) == 0) {
            perror("tht PASV address is not valid.");
        }
        int new_data_socket = socket(AF_INET, SOCK_STREAM, 0);
        
        if (connect(new_data_sock,(struct sockaddr *)&data_transfer_addr,sizeof(struct sockaddr_in)) < 0)
        {
            printf("Can't connect to server %s, port %d\n", inet_ntoa(data_transfer_addr.sin_addr),ntohs(data_transfer_addr.sin_port));
            exit(-1);
        }
        return data_transfer_sock;
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


static struct termios stored_settings;
void echo_off(void)
{
    struct termios new_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ECHO);
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}
void echo_on(void)
{
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
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
        echo_off();
        fgets(buf, sizeof(buf), stdin);
        if(buf[0]=='\n')
        {
            strncpy(pass, "anonymous", 9);
        }
        else
        {
            strncpy(pass, buf, strlen(buf)-1);
        }
        echo_on();
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
    
    
    printf("\033[32mhelp\033[0m\t--print this command list\n");
    
    printf("\033[32mlogin\033[0m\t--login\n");
    //printf("\033[32mpass\033[0m\t--transfer your password to the server\n");
    printf("\033[32mport\033[0m\t--change the current mode into PORT\n");
    printf("\033[32mpasv\033[0m\t--change the current mode into PASV\n");
    
    printf("\033[32mretr\033[0m\t--download file from the server\n");
    printf("\033[32mstor\033[0m\t--upload file to the server\n");
    printf("\033[32msyst\033[0m\t--get the system info\n");
    printf("\033[32mtype\033[0m\t--set transmission type binary or ASCII\n");
    printf("\033[32mquit\033[0m\t--quit this ftp client program\n");
    printf("\033[32mabor\033[0m\t--quit this ftp client program\n");
    
    //
    printf("\033[32mclose\033[0m\t--close the connection with the server\n");
    printf("\033[32mmkdir\033[0m\t--make new dir on the ftp server\n");
    printf("\033[32mrmdir\033[0m\t--delete the dir on the ftp server\n");
    printf("\033[32mdele\033[0m\t--delete the file on the ftp server\n");

    printf("\033[32mpwd\033[0m\t--print the current directory of server\n");
    printf("\033[32mls\033[0m\t--list the files and directoris in current directory of server\n");
    printf("\033[32mcd [directory]\033[0m\n\t--enter  of server\n");
    printf("\033[32mmode\033[0m\n\t--change current mode, PORT or PASV\n");
    printf("\033[32mput [local_file] \033[0m\n\t--send [local_file] to server as \n");
    printf("\tif  isn't given, it will be the same with [local_file] \n");
    printf("\tif there is any \' \' in , write like this \'\\ \'\n");
    printf("\033[32mget [remote file] \033[0m\n\t--get [remote file] to local host as\n");
    printf("\tif  isn't given, it will be the same with [remote_file] \n");
    printf("\tif there is any \' \' in , write like this \'\\ \'\n");
    printf("\033[32mlpwd\033[0m\t--print the current directory of local host\n");
    printf("\033[32mlls\033[0m\t--list the files and directoris in current directory of local host\n");
    printf("\033[32mlcd [directory]\033[0m\n\t--enter  of localhost\n");
    printf("\033[32mbye\033[0m\t--quit this ftp client program\n");
    printf("\033[32mquit\033[0m\t--quit this ftp client program\n");

}





