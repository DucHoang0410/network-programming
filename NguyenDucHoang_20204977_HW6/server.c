/*TCP Echo Server*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <ctype.h>

// #define PORT 5550  /* Port that will be opened */
#define BUFF_SIZE 10240
#define MES1 "Empty string!"
#define BACKLOG 2 /* Number of allowed connections */

// return digit in string
char *digitInString(char *str)
{
    int i = 0;
    char *digit = (char *)malloc(BUFF_SIZE);
    int j = 0;
    while (i < strlen(str))
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            digit[j] = str[i];
            j++;
        }
        i++;
    }
    digit[j] = '\0';
    return digit;
}
// return character in string
char *charInString(char *str)
{
    int i = 0;
    char *character = (char *)malloc(BUFF_SIZE);
    int j = 0;
    while (i < strlen(str))
    {
        if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z'))
        {
            character[j] = str[i];
            j++;
        }
        i++;
    }
    character[j] = '\0';
    return character;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }

    int server_sock, listen_sock; /* file descriptors */
    char buff[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t sin_size = sizeof(struct sockaddr_in);

    // Step 1: Construct a UDP socket
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError1: ");
        exit(0);
    }

    // Step 2: Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1])); /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = INADDR_ANY;    /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    { /* calls bind() */
        perror("\nError2: ");
        exit(0);
    }
    // Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    // Step 4: Communicate with clients
    unsigned char hash[MD5_DIGEST_LENGTH];
    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((server_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */
        FILE *file = fopen("./doremonCopy.jpg", "wb");
        while (1)
        {
            char *choice = (char *)malloc(100);
            if (choice == NULL)
            {
                perror("\nError: ");
                close(server_sock);
                return 0;
            }

            bytes_received = recv(server_sock, choice, 100, 0);
            if (bytes_received <= 0)
            {
                perror("\nError3: ");
                free(choice); // Giải phóng bộ nhớ khi không nhận được dữ liệu
                close(server_sock);
                return 0;
            }
            choice[bytes_received] = '\0';

            int dem;
            if (strlen(choice) == 1)
            {
                dem = atoi(choice);
            }

            switch (dem)
            {
            case 1:
                while (1)
                {
                    bytes_received = recv(server_sock, buff, BUFF_SIZE, 0);
                    char alpha_str[BUFF_SIZE], digit_str[BUFF_SIZE];
                    if (bytes_received < 0)
                    {
                        perror("\nError4: ");
                        close(server_sock);
                        return 1;
                    }
                    buff[bytes_received] = '\0';
                    if (bytes_received == 1)
                    {
                        bytes_sent = send(server_sock, MES1, strlen(MES1), 0);
                    }
                    else
                    {

                        printf("[%s:%d]: %s", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
                        MD5((unsigned char *)buff, strlen(buff), hash);
                        char *hash_str = (char *)malloc(SHA_DIGEST_LENGTH * 2 + 1);
                        for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
                        {
                            sprintf(&hash_str[i * 2], "%02x", hash[i]);
                        }

                        int alpha_index = 0, digit_index = 0;
                        for (int i = 0; i < strlen(hash_str); i++)
                        {
                            if (isalpha(hash_str[i]))
                            {
                                alpha_str[alpha_index++] = hash_str[i];
                            }
                            else
                            {
                                digit_str[digit_index++] = hash_str[i];
                            }
                        }
                        alpha_str[alpha_index] = '\0';
                        digit_str[digit_index] = '\0';
                        printf("\nAlpha string: %s\nDigit string: %s\n", alpha_str, digit_str);

                        bytes_sent = send(server_sock, alpha_str, strlen(alpha_str), 0); /* send to the client welcome message */
                        if (bytes_sent < 0)
                            perror("\nError5: ");

                        bytes_sent = send(server_sock, digit_str, strlen(digit_str), 0); /* send to the client welcome message */
                        if (bytes_sent < 0)
                            perror("\nError6: ");
                    }
                }
                break;
            case 2:
            printf("\nSave the file successfully\n");
              while ((bytes_received = recv(server_sock, buff, BUFF_SIZE-1, 0)) > 0) {
       				fwrite(buff, 1, bytes_received, file);
    			}
				fclose(file);
            
                break;
            }
        }
        close(server_sock);
    }
    close(listen_sock);
    return 0;
}
