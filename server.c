#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<ctype.h>
#include<pthread.h>
#include<time.h>

typedef struct flag{
	int is_SignIn;
	int is_InputUser;
	int is_PasvMode;
	int is_PortMode;
}FLAG;

void *connection_handler(void *);
void divideCmdWithParam(char *buf, char *cmd, char *param);
int is_LegalFormatIPAndPort(char *buf);
void divideIPAndPort(char *buf, char *ip, int *port);
void strip(char *str);
void strup(char *str);

void cmd_USER(int sock, char *param, FLAG *sock_flag);
void cmd_PASS(int sock, char *param, FLAG *sock_flag);
void cmd_PORT(int sock, char *param, FLAG *sock_flag, struct sockaddr_in *sock_data);
void cmd_PASV(int sock, char *param, FLAG *sock_flag, struct sockaddr_in *sock_data);
void cmd_RETR(int sock, char *param, FLAG *sock_flag, struct sockaddr_in *sock_data);
void cmd_STOR(int sock, char *param, FLAG *sock_flag, struct sockaddr_in *sock_data);
void cmd_SYST(int sock, char *param, FLAG *sock_flag);
void cmd_TYPE(int sock, char *param, FLAG *sock_flag);
void cmd_QUIT(int sock, char *param, FLAG *sock_flag);
void cmd_ABOR(int sock, char *param, FLAG *sock_flag);


int main(){
	int socket_desc, new_socket;
	int c , *new_sock;
    struct sockaddr_in server , client;
    char *message;
    
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
        return 1;
    }
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons( 21 );
    
    
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
        puts("bind failed");
        return 1;
    }
    puts("bind done");
    
    listen(socket_desc , 3);
    
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
        
        message = "220 Anonymous FTP server ready.\r\n";
        write(new_socket , message , strlen(message));
        
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0){
            perror("could not create thread");
            return 1;
        }
        
        puts("Handler assigned");
    }
    
    if (new_socket<0)
    {
        perror("accept failed");
        return 1;
    }
    
    close(socket_desc);
    return 0;
}

void *connection_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    int read_size;
    char client_message[2000];
    char cmd[100], param[100];
    FLAG sock_flag;
    struct sockaddr_in sock_data;
    
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
		client_message[read_size] = '\0';
		divideCmdWithParam(client_message, cmd, param);
		strup(cmd);
		if (strcmp(cmd, "USER") == 0){
			cmd_USER(sock, param, &sock_flag);
		}
		else if (strcmp(cmd, "PASS") == 0){
			cmd_PASS(sock, param, &sock_flag);
		}
		else if (strcmp(cmd, "PORT") == 0){
			cmd_PORT(sock, param, &sock_flag, &sock_data);
		}
		else if (strcmp(cmd, "PASV") == 0){
			cmd_PASV(sock, param, &sock_flag, &sock_data);
		}
		else if (strcmp(cmd, "RETR") == 0){
			cmd_RETR(sock, param, &sock_flag, &sock_data);
		}
		else if (strcmp(cmd, "STOR") == 0){
			cmd_STOR(sock, param, &sock_flag, &sock_data);
		}
		else if (strcmp(cmd, "SYST") == 0){
			cmd_SYST(sock, param, &sock_flag);
		}
		else if (strcmp(cmd, "TYPE") == 0){
			cmd_TYPE(sock, param, &sock_flag);
		}
		else if (strcmp(cmd, "QUIT") == 0){
			cmd_QUIT(sock, param, &sock_flag);
		}
		else if (strcmp(cmd, "ABOR") == 0){
			cmd_ABOR(sock, param, &sock_flag);
		}
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
    
    free(socket_desc);
    return 0;
}


void divideCmdWithParam(char *buf, char *cmd, char *param){
	char *tmp;
	tmp = strchr(buf, ' ');
	int i, j = 0;
	if (tmp != NULL){
		for (i = 0; i < (tmp - buf); i++){
		    cmd[i] = buf[i];
	    }
	    cmd[i++] = '\0';
	    while (buf[i] != 0){
		    param[j++] = buf[i++];
	    }
	    param[j] = '\0';
	}
	else {
		strcpy(cmd, buf);
		*param = '\0';
	}
	strip(cmd);
	strip(param);
}

void cmd_USER(int sock, char *param, FLAG *sock_flag){
	char *message;
	if (strcmp(param, "anonymous") == 0){
		message = "331 Guest login ok, send your complete e-mail address as password.\n";
		write(sock, message, strlen(message));
		sock_flag -> is_InputUser = 1;
	}
	else {
		message = "504 Support the user anonymous only\n";
		write(sock, message, strlen(message));
	}
}
void cmd_PASS(int sock, char *param, FLAG *sock_flag){
	char *message;
	if (sock_flag -> is_InputUser == 1){
		message = "230 welcome to the ftp\n";
		write(sock, message, strlen(message));
		sock_flag -> is_SignIn = 1;
	}
	else {
		message = "500 Please send USER command first\n";
		write(sock, message, strlen(message));
	}
}
void cmd_PORT(int sock, char *param, FLAG *sock_flag, struct sockaddr_in *sock_data){
	char *message;
	if (sock_flag -> is_SignIn == 0){
		message = "530 Please Sign in first\n";
		write(sock, message, strlen(message));
		return;
	}
	
	char *addr;
	int data_port = 0;
	
	addr = malloc(strlen(param));
	puts(param);
	if (is_LegalFormatIPAndPort(param)){
		puts("legal");
		divideIPAndPort(param, addr, &data_port);
		printf("%s %d\n", addr, data_port);
	}
	else {
		message = "501 Wrong format of parameter\n";
		write(sock, message, strlen(message));
		return;
	}
	
	sock_data -> sin_addr.s_addr = inet_addr(addr);
	sock_data -> sin_port = htons(data_port);
	sock_data -> sin_family = AF_INET;
	
	message = "200 PORT command successful.\n";
	write(sock, message, strlen(message));
	sock_flag -> is_PortMode = 1;
	sock_flag -> is_PasvMode = 0;
	
	free(addr);
}
void cmd_PASV(int sock, char *param, FLAG *sock_flag, struct sockaddr_in *sock_data){
	char *addr;
	int data_port;
	char *message;
	
	if (sock_flag -> is_SignIn == 0){
		message = "530 Please Sign in first\n";
		write(sock, message, strlen(message));
		return;
	}
	
	struct sockaddr_in local;
	int len = sizeof(local);
	getsockname(sock, (struct sockaddr *)&local, (socklen_t *)&len);
	addr = inet_ntoa(local.sin_addr);
	
	srand((unsigned)time(NULL));
	data_port = rand() % 45536 + 20000;
	
	memset(sock_data, 0, sizeof(*sock_data));
	sock_data -> sin_addr.s_addr = inet_addr(addr);
	sock_data -> sin_port = htons(data_port);
	sock_data -> sin_family = AF_INET;
	
	char buf[100];
	int h1,h2,h3,h4,p1,p2;
	sscanf(addr, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
	p1 = data_port / 256;
	p2 = data_port % 256;
	sprintf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\n", h1, h2, h3, h4, p1, p2);
	message = buf;
	write(sock, message, strlen(message));
	
	sock_flag -> is_PortMode = 0;
	sock_flag -> is_PasvMode = 1;
}
void cmd_RETR(int sock, char *param, FLAG *sock_flag, struct sockaddr_in *sock_data){
	char *message;
	
	if (sock_flag -> is_SignIn == 0){
		message = "530 Please Sign in first\n";
		write(sock, message, strlen(message));
		return;
	}
	
	int msg_sock, new_sock;
	if (sock_flag -> is_PortMode == 1){
		if ((msg_sock = socket(AF_INET , SOCK_STREAM , 0)) == -1){
			message = "425 Fail to create a socket\n";
			write(sock, message, strlen(message));
		    return;
		}
		if (connect(msg_sock,  (struct sockaddr *)sock_data, sizeof(*sock_data)) < 0){
			message = "425 Fail to connect\n";
			write(sock, message, strlen(message));
		    puts("14");
			return;
		}
		message = "150 PORT mode, opening the file\n";
		write(sock, message, strlen(message));
		//read file
		FILE *file;	
		char filename[100];
		sprintf(filename, "tmp/%s", param);
		file = fopen(filename, "rb");
		if (file == NULL){
			sleep(1);
		    message = "451 Could not open the file\n";
		    write(sock, message, strlen(message));
		    return;
		}
		int rc;
		char buf[1024] = {};
		while ((rc = fread(buf, 1, 1024, file)) != 0){
			if(write(msg_sock, buf, rc) <= 0){
				sleep(1);
				message = "426 Client disconnected\n";
				write(sock, message, strlen(message));
				return;
			}
		}
		fclose(file);
		close(msg_sock);
		sleep(1);
		message = "226 Transfer complete\n";
		write(sock, message, strlen(message));
		sock_flag -> is_PortMode = 0;
	}
	else if (sock_flag -> is_PasvMode == 1){
		if ((new_sock = socket(AF_INET , SOCK_STREAM , 0)) == -1){
			message = "425 Fail to create a socket\n";
			write(sock, message, strlen(message));
		    return;
		}
		if (bind(new_sock, (struct sockaddr *)sock_data, sizeof(*sock_data)) < 0){
			message = "425 Fail to connect\n";
			write(sock, message, strlen(message));
		    return;
		}
		listen(new_sock, 3);
		if ((msg_sock = accept(new_sock, NULL, NULL)) > 0){
			//read file
			message = "150 PASV mode, opening the file\n";
			write(sock, message, strlen(message));
			char filename[100];
			sprintf(filename, "tmp/%s", param);
		    FILE *file;
		    file = fopen(filename, "rb");
		    if (file == NULL){
				sleep(1);
		        message = "451 Could not open the file\n";
		        write(sock, message, strlen(message));
		        return;
		    }
		    int rc;
		    char buf[1024] = {};
		    while ((rc = fread(buf, 1, 1024, file)) != 0){
				if(write(msg_sock, buf, rc) <= 0){
					sleep(1);
					message = "426 Client disconnected\n";
					write(sock, message, strlen(message));
					return;
				}
			}
		    fclose(file);
		    close(new_sock);
		    sleep(1);
		    message = "226 Transfer complete\n";
		    write(sock, message, strlen(message));
		}
		if (msg_sock <= 0){
			message = "426 Client diconnected\n";
			write(sock, message, strlen(message));
		}
		sock_flag -> is_PasvMode = 0;
	}
	else {
		sleep(1);
		message = "502 Please set port or pasv mode first\n";
		write(sock, message, strlen(message));
		return;
	}
}
void cmd_STOR(int sock, char *param, FLAG *sock_flag, struct sockaddr_in *sock_data){
	char *message;
	
	if (sock_flag -> is_SignIn == 0){
		message = "530 Please Sign in first\n";
		write(sock, message, strlen(message));
		return;
	}
	
	int msg_sock, new_sock;
	if (sock_flag -> is_PortMode == 1){
		if ((msg_sock = socket(AF_INET , SOCK_STREAM , 0)) == -1){
			message = "425 Fail to create a socket\n";
			write(sock, message, strlen(message));
		    return;
		}
		if (connect(msg_sock, (struct sockaddr *)sock_data, sizeof(*sock_data)) < 0){
			message = "425 Fail to connect\n";
			write(sock, message, strlen(message));
		    return;
		}
		message = "150 PORT mode, opening the file\n";
		write(sock, message, strlen(message));
		//read file;	
		char filename[100];
		sprintf(filename, "tmp/%s", param);
		FILE *file;
		file = fopen(filename, "wb");
		if (file == NULL){
			sleep(1);
		    message = "451 Could not open the file\n";
		    write(sock, message, strlen(message));
		    return;
		}
		int rc;
		char buf[1024] = {};
		while ((rc = read(msg_sock, buf, 1024)) > 0){
			fwrite(buf, 1, rc, file);
			memset(buf, 0, sizeof(buf));
		}
		if (rc < 0){
			sleep(1);
			message = "426 Client diconnected\n";
			write(sock, message, strlen(message));
			return;
		}
		fclose(file);
		close(msg_sock);
		sleep(1);
		message = "226 Transfer complete\n";
		write(sock, message, strlen(message));
		sock_flag -> is_PortMode = 0;
	}
	else if (sock_flag -> is_PasvMode == 1){
		if ((new_sock = socket(AF_INET , SOCK_STREAM , 0)) == -1){
			message = "425 Fail to create a socket\n";
			write(sock, message, strlen(message));
		    return;
		}
		if (bind(new_sock, (struct sockaddr *)sock_data, sizeof(*sock_data)) < 0){
			message = "425 Fail to connect\n";
			write(sock, message, strlen(message));
		    return;
		}
		listen(new_sock, 3);
		if ((msg_sock = accept(new_sock, NULL, NULL)) <= 0){
			message = "426 Client diconnected\n";
			write(sock, message, strlen(message));
			return;
		}
		//read file
		message = "150 PASV mode, opening the file\n";
		write(sock, message, strlen(message));;	
		char filename[100];
		sprintf(filename, "tmp/%s", param);
		sprintf(filename, "tmp/%s", param);
		FILE *file;
		file = fopen(filename, "wb");
		if (file == NULL){
			sleep(1);
		    message = "451 Could not open the file\n";
		    write(sock, message, strlen(message));
		    return;
		}
		int rc;
		char buf[1024] = {};
		while ((rc = read(msg_sock, buf, 1024)) > 0){
			fwrite(buf, 1, rc, file);
			memset(buf, 0, sizeof(buf));
		}
		if (rc < 0){
			sleep(1);
			message = "426 Client diconnected\n";
			write(sock, message, strlen(message));
			return;
		}
		fclose(file);
		close(new_sock);
		sleep(1);
		message = "226 Transfer complete\n";
		write(sock, message, strlen(message));
		sock_flag -> is_PasvMode = 0;
	}
	else {
		sleep(1);
		message = "502 Please set port or pasv mode first\n";
		write(sock, message, strlen(message));
		return;
	}
}
void cmd_SYST(int sock, char *param, FLAG *sock_flag){
	char *message;
	
	if (sock_flag -> is_SignIn == 0){
		message = "530 Please Sign in first\n";
		write(sock, message, strlen(message));
		return;
	}
	
	message = "215 UNIX Type: L8\n";
	write(sock, message, strlen(message));
}
void cmd_TYPE(int sock, char *param, FLAG *sock_flag){
	char *message;
	
	if (sock_flag -> is_SignIn == 0){
		message = "530 Please Sign in first\n";
		write(sock, message, strlen(message));
		return;
	}
	
	if (strcmp(param, "I") == 0){
		message = "200 Type set to I.\n";
		write(sock, message, strlen(message));
	}
}
void cmd_QUIT(int sock, char *param, FLAG *sock_flag){
	char *message;
	
	if (sock_flag -> is_SignIn == 0){
		message = "530 Please Sign in first\n";
		write(sock, message, strlen(message));
		return;
	}
	
	message = "221 Goodbye.\n";
	write(sock, message, strlen(message));
	close(sock);
}
void cmd_ABOR(int sock, char *param, FLAG *sock_flag){
	char *message;
	
	if (sock_flag -> is_SignIn == 0){
		message = "530 Please Sign in first\n";
		write(sock, message, strlen(message));
		return;
	}
	
	message = "221 Goodbye.\n";
	write(sock, message, strlen(message));
	close(sock);
}

void divideIPAndPort(char *buf, char *ip, int *port){
	int count = 0;
	int i = 0, j = 0;
	int mode = 0;
	char *p = buf;
	char *strport = malloc(strlen(buf));
	while(*p != 0){
		if (mode == 0){
			if (*p == ','){
				count++;
				if (count != 4){
				    ip[i++] = '.';
				}
				else {
					mode = 1;
					ip[i] = '\0';
				}
			}
			else {
				ip[i++] = *p;
			}
		}
		else if (mode == 1){
			strport[j++] = *p;
		}
		p++;
	}
	strport[j] = '\0';
	int p1, p2;
	sscanf(strport, "%d,%d", &p1, &p2);
	*port = p1 * 256 + p2;
	free(strport);
}

int is_LegalFormatIPAndPort(char *buf){
	char *p = buf;
	int count = 0;
	while (*p != 0){
		if (*p == ','){
			count++;
			if(*(p+1) == ','){
				printf("double ,\n");
				return 0;
			}
			p++;
		}
		else if (*p >= '0' && *p <= '9'){
			p++;
		}
		else {
			printf("other ch\n");
			return 0;
		}
	}
	if (count != 5){
		printf("more ,\n");
		return 0;
	}
	else return 1;
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
