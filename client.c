#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>

void cmd_USER(int sock);
void cmd_PASS(int sock);
void cmd_PORT(int sock, int *mode, struct sockaddr_in *sock_data);
void cmd_PASV(int sock, int *mode, struct sockaddr_in *sock_data);
void cmd_RETR(int sock, int *mode, struct sockaddr_in *sock_data);
void cmd_STOR(int sock, int *mode, struct sockaddr_in *sock_data);
void cmd_SYST(int sock);
void cmd_TYPE(int sock);
void cmd_QUIT(int sock);
void cmd_ABOR(int sock);
void strip(char *str);
void strup(char *str);

int main(){
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
	
	printf("now you can choose the following commands:\n");
	printf("user | pass | port | pasv | retr | stor | syst | type | quit | abor\n");
	char cmd[100];
	int is_quit = 0;
	int mode = 0;
	struct sockaddr_in sock_data;
	while (is_quit == 0){
		fflush(stdin);
		scanf("%s", cmd);
		strup(cmd);
		if (strcmp(cmd, "USER") == 0){
			cmd_USER(socket_desc);
		}
		else if (strcmp(cmd, "PASS") == 0){
			cmd_PASS(socket_desc);
		}
		else if (strcmp(cmd, "PORT") == 0){
			cmd_PORT(socket_desc, &mode, &sock_data);
		}
		else if (strcmp(cmd, "PASV") == 0){
			cmd_PASV(socket_desc, &mode, &sock_data);
		}
		else if (strcmp(cmd, "RETR") == 0){
			cmd_RETR(socket_desc, &mode, &sock_data);
		}
		else if (strcmp(cmd, "STOR") == 0){
			cmd_STOR(socket_desc, &mode, &sock_data);
		}
		else if (strcmp(cmd, "SYST") == 0){
			cmd_SYST(socket_desc);
		}
		else if (strcmp(cmd, "TYPE") == 0){
			cmd_TYPE(socket_desc);
		}
		else if (strcmp(cmd, "QUIT") == 0){
			cmd_QUIT(socket_desc);
			is_quit = 1;
		}
		else if (strcmp(cmd, "ABOR") == 0){
			cmd_ABOR(socket_desc);
			is_quit = 1;
		}
	}
	
}

void strip(char *str)
{
    char *start = str - 1;
    char *end = str;
    char *p = str;
    while(*p)
    {
        switch(*p)
        {
        case ' ':
        case '\r':
        case '\n':
            {
                if(start + 1==p)
                    start = p;
            }
            break;
        default:
            break;
        }
        ++p;
    }
    
    --p;
    ++start;
    if(*start == 0)
    {
        *str = 0 ;
        return;
    }
    end = p + 1;
    while(p > start)
    {
        switch(*p)
        {
        case ' ':
        case '\r':
        case '\n':
            {
                if(end - 1 == p)
                    end = p;
            }
            break;
        default:
            break;
        }
        --p;
    }
    memmove(str,start,end-start);
    *(str + (end - start)) = 0;
}

void strup(char *str){
	int i = 0;
	while(str[i] != 0){
		str[i] = toupper(str[i]);
		i++;
	}
}

void cmd_USER(int sock){
	int size;
	char username[100], buf[100], reply[1000];
	scanf("%s", username);
	strip(username);
	sprintf(buf, "USER %s\r\n", username);
	if (write(sock, buf, strlen(buf)) < 0){
		puts("send failed");
	}
	if ((size = read(sock, reply, 1000)) < 0){
		puts("receive failed");
	}
	reply[size] = '\0';
	printf("%s", buf);
	printf("%s", reply);
}
void cmd_PASS(int sock){
	int size;
	char pwd[100], buf[100], reply[1000];
	scanf("%s", pwd);
	strip(pwd);
	sprintf(buf, "PASS %s\r\n", pwd);
	if (write(sock, buf, strlen(buf)) < 0){
		puts("send failed");
	}
	if (((size = read(sock, reply, 1000)) < 0)){
		puts("receive failed");
	}
	reply[size] = '\0';
	printf("%s", buf);
	printf("%s", reply);
}
void cmd_PORT(int sock, int *mode, struct sockaddr_in *sock_data){
	char *addr;
	int h1,h2,h3,h4,p1,p2;
	struct sockaddr_in local;
	int len = sizeof(local);
	getsockname(sock, (struct sockaddr *)&local, (socklen_t *)&len);
	addr = inet_ntoa(local.sin_addr);
	sscanf(addr, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
	
	int size;
	int port;
	char buf[100], reply[1000];
	scanf("%d", &port);
	p1 = port / 256;
	p2 = port % 256;
	
	sock_data -> sin_addr.s_addr = inet_addr(addr);
	sock_data -> sin_port = htons(port);
	sock_data -> sin_family = AF_INET;
	
	*mode = 1; //port
	sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", h1, h2, h3, h4, p1, p2);
	if (write(sock, buf, strlen(buf)) < 0){
		puts("send failed");
	}
	if (((size = read(sock, reply, 1000)) < 0)){
		puts("receive failed");
	}
	reply[size] = '\0';
	printf("%s", buf);
	printf("%s", reply);
}
void cmd_PASV(int sock, int *mode, struct sockaddr_in *sock_data){
	int size;
	char *buf, reply[1000];
	buf = "PASV\r\n";
	if (write(sock, buf, strlen(buf)) < 0){
		puts("send failed");
	}
	if (((size = read(sock, reply, 1000)) < 0)){
		puts("receive failed");
	}
	reply[size] = '\0';
	printf("%s", buf);
	printf("%s", reply);
	
	int h1,h2,h3,h4,p1,p2;
	int port;
	char addr[100];
	
	strip(reply);
	sscanf(reply, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2);
	sprintf(addr, "%d.%d.%d.%d", h1, h2, h3, h4);
	port = 256 * p1 + p2;
	
	sock_data -> sin_addr.s_addr = inet_addr(addr);
	sock_data -> sin_port = htons(port);
	sock_data -> sin_family = AF_INET;
	
	*mode = 2; //pasv
	
}
void cmd_RETR(int sock, int *mode, struct sockaddr_in *sock_data){
	int msg_sock, new_sock;
	int size;
	char filename[100], buf[100], reply[1000];
	scanf("%s", filename);
	strip(filename);
	sprintf(buf, "RETR %s\r\n", filename);
	printf("mode %d\n", *mode);
	if ((*mode) == 1){
		puts("1");
		if ((new_sock = socket(AF_INET , SOCK_STREAM , 0)) == -1){
			puts("Fail to create a socket");
			return;
		}
		if (bind(new_sock, (struct sockaddr *)sock_data, sizeof(*sock_data)) < 0){
			puts("425 Fail to bind");
			return;
		}
		listen(new_sock, 3);
		puts("listening");
	    if (write(sock, buf, strlen(buf)) < 0){
		    puts("send failed");
	    }
	    printf("%s", buf);
	    //connected
		if ((msg_sock = accept(new_sock, NULL, NULL)) > 0){
			if (((size = read(sock, reply, 1000)) < 0)){
				puts("recv failed");
			}
			reply[size] = '\0';
			puts("connected");
			printf("%s", reply);
			
			FILE *file;
		    file = fopen(filename, "wb");
		    if (file == NULL){
		        puts("Could not open the file\n");
		        return;
		    }
		    int rc;
		    char buf[1024] = {};
		    while ((rc = read(msg_sock, buf, 1024)) != 0){
				fwrite(buf, 1, rc, file);
				memset(buf, 0 ,sizeof(buf));
			}
		    fclose(file);
		    close(msg_sock);
		    if (((size = read(sock, reply, 1000)) < 0)){
				puts("recv failed");
			}
			reply[size] = '\0';
			printf("%s", reply);
			*mode = 0;
		}
	}
	else if (*mode == 2){
		if (write(sock, buf, strlen(buf)) < 0){
		    puts("send failed");
	    }
	    printf("%s", buf);
	    
		if ((msg_sock = socket(AF_INET , SOCK_STREAM , 0)) == -1){
			puts("Fail to create a socket");
			return;
		}
		if (connect(msg_sock, (struct sockaddr *)sock_data, sizeof(*sock_data)) < 0){
			puts("425 Fail to connect");
			return;
		}
		//connected
		if (((size = read(sock, reply, 1000)) < 0)){
			puts("recv failed");
		}
		reply[size] = '\0';
		puts("connected");
		printf("%s", reply);
		
		FILE *file;
		file = fopen(filename, "wb");
		if (file == NULL){
		    puts("Could not open the file\n");
		    return;
		}
		puts("1");
		int rc;
		char buf[1024] = {};
		while ((rc = read(msg_sock, buf, 1024)) != 0){
			fwrite(buf, 1, rc, file);
			memset(buf, 0 ,sizeof(buf));
		}
		fclose(file);
		puts("2");
		close(msg_sock);
		if (((size = read(sock, reply, 1000)) < 0)){
			puts("recv failed");
		}
		reply[size] = '\0';
		printf("%s", reply);
		
		puts("3");
		*mode = 0;
	}
	else {
		if (((size = read(sock, reply, 1000)) < 0)){
			puts("recv failed");
		}
		reply[size] = '\0';
		printf("%s", reply);
		*mode = 0;
	}
}
void cmd_STOR(int sock, int *mode, struct sockaddr_in *sock_data){
	int msg_sock, new_sock;
	int size;
	char filename[100], buf[100], reply[1000];
	scanf("%s", filename);
	strip(filename);
	sprintf(buf, "STOR %s\r\n", filename);
	printf("mode %d\n", *mode);
	if ((*mode) == 1){
		puts("1");
		if ((new_sock = socket(AF_INET , SOCK_STREAM , 0)) == -1){
			puts("Fail to create a socket");
			return;
		}
		if (bind(new_sock, (struct sockaddr *)sock_data, sizeof(*sock_data)) < 0){
			puts("425 Fail to bind");
			return;
		}
		listen(new_sock, 3);
		puts("listening");
	    if (write(sock, buf, strlen(buf)) < 0){
		    puts("send failed");
	    }
	    printf("%s", buf);
	    //connected
		if ((msg_sock = accept(new_sock, NULL, NULL)) > 0){
			if (((size = read(sock, reply, 1000)) < 0)){
				puts("recv failed");
			}
			reply[size] = '\0';
			puts("connected");
			printf("%s", reply);
			
			FILE *file;
		    file = fopen(filename, "rb");
		    if (file == NULL){
		        puts("Could not open the file\n");
		        return;
		    }
		    int rc;
		    char buf[1024] = {};
		    while ((rc = fread(buf, 1, 1024, file)) != 0){
			    if(write(msg_sock, buf, rc) <= 0){
				    puts("426 Client diconnected");
				    return;
			    }
				memset(buf, 0 ,sizeof(buf));
		    }
		    fclose(file);
		    close(msg_sock);
		    if (((size = read(sock, reply, 1000)) < 0)){
				puts("recv failed");
			}
			reply[size] = '\0';
			printf("%s", reply);
			*mode = 0;
		}
	}
	else if (*mode == 2){
		if (write(sock, buf, strlen(buf)) < 0){
		    puts("send failed");
	    }
	    printf("%s", buf);
	    
		if ((msg_sock = socket(AF_INET , SOCK_STREAM , 0)) == -1){
			puts("Fail to create a socket");
			return;
		}
		if (connect(msg_sock, (struct sockaddr *)sock_data, sizeof(*sock_data)) < 0){
			puts("425 Fail to connect");
			return;
		}
		//connected
		if (((size = read(sock, reply, 1000)) < 0)){
			puts("recv failed");
		}
		reply[size] = '\0';
		puts("connected");
		printf("%s", reply);
		
		FILE *file;
		file = fopen(filename, "rb");
		if (file == NULL){
		    puts("Could not open the file\n");
		    return;
		}
		int rc;
		char buf[1024] = {};
		while ((rc = fread(buf, 1, 1024, file)) != 0){
			if(write(msg_sock, buf, rc) <= 0){
				puts("426 Client diconnected");
				return;
			}
			memset(buf, 0 ,sizeof(buf));
		}
		fclose(file);
		close(msg_sock);
		if (((size = read(sock, reply, 1000)) < 0)){
			puts("recv failed");
		}
		reply[size] = '\0';
		printf("%s", reply);
		*mode = 0;
	}
	else {
		if (((size = read(sock, reply, 1000)) < 0)){
			puts("recv failed");
		}
		reply[size] = '\0';
		printf("%s", reply);
		*mode = 0;
	}
}
void cmd_SYST(int sock){
	int size;
	char *buf, reply[1000];
	buf = "SYST\r\n";
	if (write(sock, buf, strlen(buf)) < 0){
		puts("send failed");
	}
	if (((size = read(sock, reply, 1000)) < 0)){
		puts("receive failed");
	}
	reply[size] = '\0';
	printf("%s", buf);
	printf("%s", reply);
}
void cmd_TYPE(int sock){
	int size;
	char param[100], buf[100], reply[1000];
	scanf("%s", param);
	sprintf(buf, "TYPE %s\r\n", param);
	if (write(sock, buf, strlen(buf)) < 0){
		puts("send failed");
	}
	if (((size = read(sock, reply, 1000)) < 0)){
		puts("receive failed");
	}
	reply[size] = '\0';
	printf("%s", buf);
	printf("%s", reply);
}
void cmd_QUIT(int sock){
	int size;
	char *buf, reply[1000];
	buf = "QUIT\r\n";
	if (write(sock, buf, strlen(buf)) < 0){
		puts("send failed");
	}
	if (((size = read(sock, reply, 1000)) < 0)){
		puts("receive failed");
	}
	reply[size] = '\0';
	printf("%s", buf);
	printf("%s", reply);
}
void cmd_ABOR(int sock){
	int size;
	char *buf, reply[1000];
	buf = "QUIT\r\n";
	if (write(sock, buf, strlen(buf)) < 0){
		puts("send failed");
	}
	if (((size = read(sock, reply, 1000)) < 0)){
		puts("receive failed");
	}
	reply[size] = '\0';
	printf("%s", buf);
	printf("%s", reply);
}
