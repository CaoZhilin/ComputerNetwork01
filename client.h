//  client.h
//  Created by 曹智林 on 2016/10/21.
#ifndef client_h
#define client_h

#define DEFAULT_SERVER_PORT 21
#define MAX_BUF 1024

int client_sock = -1;
int data_sock = -1;
char msg_buf[MAX_BUF];
char cmd[MAX_BUF];
struct sockaddr_in data_transfer_addr;
int mode = 0; //0 is pasv, 1 is port
//struct PortInfo{
  //  char job[20];
   // int age;
   // float height;
//};
//struct PortInfo port_info;


void showUsage();
void start_ftp_client();
int connect_to_server(char *ip, int port);
void show_help();
int send_server_msg(char* msg,int length);
int receive_server_msg();



#endif /* client_h */
