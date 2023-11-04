/*UDP Echo Client*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// #define SERV_PORT 5550
// #define SERV_IP "127.0.0.1"
#define BUFF_SIZE 1024
#define MES1 "Empty string!"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }

    int client_sock;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr;
    int bytes_sent, bytes_received;
    socklen_t sin_size = sizeof(struct sockaddr);

    // Step 1: Construct a UDP socket
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { /* calls socket() */
        perror("\nError: ");
        exit(0);
    }

    // Step 2: Define the address of the server
    int portNumber = atoi(argv[2]);
    char *ipAdrr = argv[1];
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNumber);
    server_addr.sin_addr.s_addr = inet_addr(ipAdrr);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit imediately! ");
        return 0;
    }
    // Step 4: Communicate with server
    while (1)
    {
        sin_size = sizeof(struct sockaddr);
        FILE *file = fopen("./doremon.jpg", "rb");
        while (1)
        {
            int choice;
            printf("Menu:\n");
            printf("----------------------------------\n");
            printf("1. Gửi xâu bất kỳ.\n");
            printf("2. Gửi nội dung một file.\n");
            printf("----------------------------------\n");
            scanf("%d", &choice);
            getchar();
            char choice_str[10];
            sprintf(choice_str, "%d", choice);
            bytes_sent = send(client_sock, choice_str, strlen(choice_str), 0);
            if (bytes_sent < 0)
            {
                perror("Error: ");
                close(client_sock);
                return 0;
            }
            switch (choice)
            {
            case 1:
                while (1)
                {
                    printf("\nInsert string to send: ");
                    memset(buff, '\0', (strlen(buff) + 1));
                    fgets(buff, BUFF_SIZE, stdin);

                    bytes_sent = send(client_sock, buff, strlen(buff), 0);
                    if (bytes_sent < 0)
                    {
                        perror("Error: ");
                        close(client_sock);
                        return 0;
                    }

                    bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
                    if (bytes_received < 0)
                    {
                        perror("Error: ");
                        close(client_sock);
                        return 0;
                    }
                    buff[bytes_received] = '\0';
                    if (strcmp(buff, MES1) == 0)
                    {
                        printf("Empty string!!!\n");
                        close(client_sock);
                        return 0;
                    }
                    printf("Reply from server: %s\n", buff);
                }
                break;
            case 2:
        
				if (!file) {
        			perror("Error: ");
       				return -1;
    			}
				while (!feof(file)) {
       				size_t bytesRead = fread(buff, 1, sizeof(buff), file);
        			if (send(client_sock, buff, bytesRead, 0) < 0) {
            			perror("Error: ");
            			return -1;
        			}
        			bzero(buff, BUFF_SIZE);
				}
				fclose(file);
                break;
            }
        }
    }
    close(client_sock);
    return 0;
}
