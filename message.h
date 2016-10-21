#ifndef __SERVER_H_
#define __SERVER_H_

#define MAX_BUF 1024
#define SERVER_PORT 21
#define BACKLOG 10      //MAX_USER_NUM
#define MAX_USER_NUM 10

#define PORT_SUCCESS         "200 PORT command successful.\r\n"
#define TYPE_I_SUCCESS         "200 Type set to I.\r\n"

char msg_buf[MAX_BUF]={0};
char ftp_path[MAX_BUF]={0};     // root
char pwd[MAX_BUF] = {0};        //current directory
int port = 0;

char default_user[MAX_USER_NUM][MAX_BUF];
char default_pwd[MAX_USER_NUM][MAX_BUF];
int default_user_amount = 0;
long bytesOfDownload = 0, bytesOfUpload = 0;
long numOfDownloadFiles = 0, numOfUploadFiles = 0;

//struct sockaddr_in server;
struct sockaddr_in data_transfer_addr;

//int serverFD;
int ftp_data_sock = -1;
int connect_data_sock = -1;
int connect_mode = 0;   // 0 means PASV while 1 means PORT

void process_terminal_arguments(int argc, char* argv[]);
void client_request_handler(int clientFD, struct sockaddr_in client);
int send_client_message(int clientFD,char* msg,int length);
int receive_client_msg(int clientFD);
int login(int clientFD);
void load_in_default_users();
void get_ip(char *buffer,char *ip_address,int *port);
void Pasv(int clientFD, struct sockaddr_in client);
void Port(int clientFD);
void Retr(int clientFD);
void Stor(int clientFD);
int mkdir_r(const char *path);

char msgPortSuc200[]="200 PORT command successful.\r\n";
char msgServerReady220[]="220 Anonymous FTP server ready.\r\n";
char msgWelcome230[]="230-\r\n230-Welcome to czl's FTP\r\n230-\r\n230-This FTP program is made by czl.\r\n230-Use in violation of any applicable laws is strictly\r\n230-prohibited. We make no guarantees, explicit or implicit, about the\r\n230-contents of this site. Use at your own risk.\r\n230-\r\n230 User logged in, proceed.\r\n";
char serverMsgAnms331[]="331 User name okay, send your complete e-mail address as password.\r\n";
char msgInputPassword331[]="331 We've got your username,please input your password and make other request later :) \r\n";
char serverMsg221[]="221-Thank you for using Vince's FTP\r\n221 Goodbye!\r\n";
char msgRetr150[]="150 Opening BINARY mode data connection for ";
char msgStor150[]="150 Opening BINARY mode data connection for ";
char msgTranComp226[]="226 Transfer complete.\r\n";

char serverMsg213[]="213 File status.\r\n";
char serverMsg211[]="211 System status, or system help reply.\r\n";
char serverMsg350[]="350 Requested file action pending further information.\r\n";
char msgNotLoggedIn530[]="530 Not logged in.\r\n";
char msgNotImplemented202[]="202 Command not implemented, superfluous at this site.\r\n";
char msgInputUsername202[]="202 Please Input your username firt.\r\n"
char msgOpenConnectionF425[]="425 Can't open data connection.\r\n";
char serverMsg426[]="426 Connection closed, transfer aborted.\r\n";
char msgReadFailed451[]="451 Requested action aborted, error in reading file from disk.\r\n";
char msgWriteFailed451[]="451 Requested action aborted, error in writing file.\r\n";
#endif



const char *ServerReady220 = "220 Anonymous FTP server ready.\r\n";
