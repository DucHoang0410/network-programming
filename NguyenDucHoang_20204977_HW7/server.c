#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
// #define PORT 5550   /* Port that will be opened */
#define BACKLOG 5 /* Number of allowed connections */
#define BUFF_SIZE 1024
#define MES1 "Account is not active!"
#define MES2 "Account is blocked!"
#define MES3 "Not OK"
#define MES4 "OK"
#define MES5 "Account not found!!!"
#define  EXIT_SUCCESS 0
#define EXIT_FAILURE 1
char tenFile[100] = "./account.txt";
int status;
char *password;
char login[50];

struct User
{
    char userName[50];
    char password[50];
    int status;
    struct User *next;
};

struct User *head = NULL;
struct User *current = NULL;

void readFile()
{
    FILE *f;
    f = fopen(tenFile, "r");
    if (f == NULL)
    {
        printf("File NULL!\n");
    }

    while (!feof(f))
    {
        struct User *ptr = (struct User *)malloc(sizeof(struct User));
        fscanf(f, "%s %s %d\n", ptr->userName, ptr->password, &ptr->status);
        ptr->next = head;
        head = ptr;
    }
    fclose(f);
}

void writeFile()
{
    FILE *f;
    f = fopen(tenFile, "w");
    struct User *ptr = head;
    while (ptr != NULL)
    {
        fprintf(f, "%s %s %d\n", ptr->userName, ptr->password, ptr->status);
        ptr = ptr->next;
    }
    fclose(f);
}

void printList()
{
    struct User *ptr = head;
    printf("\n[ ");
    while (ptr != NULL)
    {
        printf("%s %s %d || ", ptr->userName, ptr->password, ptr->status);
        ptr = ptr->next;
    }
    printf(" ]\n");
}

int searchUserName(char name[])
{
    if (head == NULL)
    {
        return 0;
    }
    struct User *link = head;
    while (link != NULL)
    {
        if (strcmp(link->userName, name) == 0)
        {
            current = link;
            return 1;
        }
        link = link->next;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s \n", argv[0]);
        exit(1);
    }
    readFile();
	printList();
    int listen_sock, conn_sock; /* file descriptors */
    char recv_data[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    int sin_size;

    // Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError1: ");
        return 0;
    }

    // Step 2: Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));     /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { /* calls bind() */
        perror("\nError2: ");
        return 0;
    }

    // Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError3: ");
        return 0;
    }

    // Step 4: Communicate with client
    while (1)
    {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError4: ");

        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

        // start conversation
        pid_t pid = fork();

        if (pid == 0)
        { // Child process
            close(listen_sock);

            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE, 0);
            if (bytes_received < 0)
            {
                perror("\nError5: ");
                close(conn_sock);
                return 0;
            }
            recv_data[bytes_received - 1] = '\0';
            printf(">>>>recv_data %s\n",recv_data);

            char *userName = strdup(recv_data);
            int check = searchUserName(userName);
            printf(">>>>check %d\n",check);
            if (check == 1)
            {
                if (current->status == 0)
                {
                    bytes_sent = send(conn_sock, MES2, strlen(MES2), 0);
                }
                else
                {
                    bytes_sent = send(conn_sock, MES3, strlen(MES3), 0);
                    bytes_received = recv(conn_sock, recv_data, BUFF_SIZE, 0);
                    if (bytes_received < 0)
                    {
                        perror("\nError6: ");
                        close(conn_sock);
                        return 0;
                    }
                    recv_data[bytes_received - 1] = '\0';
                    printf("%s", recv_data);
                    char *password = strdup(recv_data);
                    int count = 1;
                    if (strcmp(current->password, password) != 0)
                    {
                        while (strcmp(current->password, password) != 0 && count < 3)
                        {
                            bytes_sent = send(conn_sock, MES3, strlen(MES3), 0);
                            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE, 0);
                            if (bytes_received < 0)
                            {
                                perror("\nError7: ");
                                close(conn_sock);
                                return 0;
                            }
                            recv_data[bytes_received - 1] = '\0';
                            char *x = strdup(recv_data);
                            strcpy(password, x);
                            count++;
                        }
                        if (strcmp(current->password, password) != 0 && count == 3)
                        {
                            bytes_sent = send(conn_sock, MES2, strlen(MES2), 0);
                            current->status = 0;
                            int i = remove(tenFile);
                            writeFile();
                        }
                    }
                    if (strcmp(current->password, password) == 0)
                    {
                        bytes_sent = send(conn_sock, MES4, strlen(MES4), 0);
                        
                            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE, 0);
                            if (bytes_received < 0)
                            {
                                perror("\nError8: ");
                                close(conn_sock);
                                return 0;
                            }
                            recv_data[bytes_received] = '\0';
                            char *checkPassword = strdup(recv_data);
                            if (strcmp(checkPassword, "bye") == 0)
                            {
                                char noti[] = "Goodbye ";
                                strcat(noti, current->userName);
                                bytes_sent = send(conn_sock, noti, strlen(noti), 0);
                        
                            }
                        
                    }
                }
            }
            else
            {
                bytes_sent = send(conn_sock, MES5, strlen(MES5), 0);
                return 0;
            }

            exit(EXIT_SUCCESS);   
        }
        else if (pid > 0)
        {
            close(conn_sock);
        }
        else
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }
    close(listen_sock);
    return 0;
}