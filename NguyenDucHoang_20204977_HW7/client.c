#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

// #define SERVER_ADDR "127.0.0.1"
// #define SERVER_PORT 5550
#define BUFF_SIZE 1024
#define MES1 "Account is not active!"
#define MES2 "Account is blocked!"
#define MES3 "Not OK"
#define MES4 "OK"
#define MES5 "Account not found!!!"
int main(int argc, char *argv[]){

	 if (argc != 3)
    {
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }
	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr; /* server's address information */
	int  bytes_sent, bytes_received;
	socklen_t sin_size = sizeof(struct sockaddr);

	//Step 1: Construct socket
	if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { /* calls socket() */
        perror("\nError: ");
        exit(0);
    }
	
	//Step 2: Specify server address
	int portNumber = atoi(argv[2]);
	char *ipAdrr = argv[1];
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portNumber);
	server_addr.sin_addr.s_addr = inet_addr(ipAdrr);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	//Step 4: Communicate with server			
	while(1){
		//send message
		printf("\nUser Name:");
		memset(buff,'\0',(strlen(buff)+1));
		fgets(buff, BUFF_SIZE, stdin);
		bytes_sent = send(client_sock, buff, strlen(buff), 0);
		if(bytes_sent < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
		if(bytes_received < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		buff[bytes_received]= '\0';

		char *check = strdup(buff);
		if(strcmp(check,MES2 ) == 0 ){
			printf("%s\n", MES2);
		} 
        if(strcmp(check, MES5)==0){
			printf("%s\n", MES5);
		}

		while(strcmp(check, MES3) == 0){
			printf("Insert Password:\n");
			memset(buff,'\0',(strlen(buff)+1));
			fgets(buff, BUFF_SIZE, stdin);
			bytes_sent = send(client_sock, buff, strlen(buff), 0);
			bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
			if(bytes_received < 0){
				perror("Error: ");
				close(client_sock);
				return 0;
			}
			buff[bytes_received]= '\0';
			char *check1 = strdup(buff);
			strcpy(check,check1);
			printf("%s\n",check);
		}

		if(strcmp(check,MES4) == 0){
			printf("Enter bye to log out.\n");
			fgets(buff, BUFF_SIZE, stdin);
			size_t len = strcspn(buff, "\n");
            if (len < sizeof(buff) - 1) {
            buff[len] = '\0';
            } 

			if(strcmp(buff,"bye") == 0){
            bytes_sent = send(client_sock, buff, strlen(buff), 0);
			bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
			if(bytes_received < 0){
				perror("Error: ");
				close(client_sock);
				return 0;
			}
			buff[bytes_received]= '\0';
			printf("%s\n", buff);
			return 0;  
			}
		}	
		if(strcmp(check,MES2)==0){
			return 0;
		}	
	}	
	
	//Step 4: Close socket
	close(client_sock);
	return 0;
}
