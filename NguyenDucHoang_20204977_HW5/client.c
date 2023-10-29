/*UDP Echo Client*/
#include <stdio.h>       
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include<stdlib.h>
#include <unistd.h>

// #define SERV_PORT 5550
// #define SERV_IP "127.0.0.1"
#define BUFF_SIZE 1024
#define MES7 "account not found"
#define MES3 "Not OK"
#define MES2 "Account is blocked!"

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }

	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent,bytes_received;
	socklen_t sin_size = sizeof(struct sockaddr);
	
	//Step 1: Construct a UDP socket
	if ((client_sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){  /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	//Step 2: Define the address of the server
    int portNumber = atoi(argv[2]);
    char *ipAdrr = argv[1];
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portNumber);
	server_addr.sin_addr.s_addr = inet_addr(ipAdrr);
	
	//Step 3: Communicate with server
	while(1){
		printf("\nUser Name:");
		memset(buff,'\0',(strlen(buff)+1));
		fgets(buff, BUFF_SIZE, stdin);
		bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
		if(bytes_sent < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		bytes_received = recvfrom(client_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &server_addr, &sin_size);
		if(bytes_received < 0){
			perror("Error: ");
			close(client_sock);
			return 0;
		}
		// printf("abc\n");
		buff[bytes_received]= '\0';
		char *check = strdup(buff);
		if(strcmp(check,MES2 ) == 0 ){
			printf("%s\n", MES2);
		} 
        if(strcmp(check, MES7)==0){
			printf("%s\n", MES7);
		}

		while(strcmp(check, MES3) == 0){
			printf("Insert Password:\n");
			memset(buff,'\0',(strlen(buff)+1));
			fgets(buff, BUFF_SIZE, stdin);
			bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
			bytes_received = recvfrom(client_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &server_addr, &sin_size);
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
		if(strcmp(check,"OK") == 0){
			while(1){
				printf("Insert New Password:\n");
			fgets(buff, BUFF_SIZE, stdin);

			size_t len = strcspn(buff, "\n");
            if (len < sizeof(buff) - 1) {
            buff[len] = '\0';
            } 

			if(strcmp(buff,"bye") == 0){
                bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
			bytes_received = recvfrom(client_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &server_addr, &sin_size);
			if(bytes_received < 0){
				perror("Error: ");
				close(client_sock);
				return 0;
			}
			buff[bytes_received]= '\0';
			printf("%s\n", buff);
			return 0;  
			}

			bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
			bytes_received = recvfrom(client_sock, buff, BUFF_SIZE, 0, (struct sockaddr *) &server_addr, &sin_size);
			if(bytes_received < 0){
				perror("Error: ");
				close(client_sock);
				return 0;
			}
			buff[bytes_received]= '\0';
			printf("%s\n", buff);

			}
		
		}
		if(strcmp(check,"Account is blocked!")==0){
			return 0;
		}	
	}	
	close(client_sock);
	return 0;
}
