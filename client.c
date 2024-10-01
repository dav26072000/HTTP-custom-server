#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVER_PORT "8080"    // The port the server is listening on
#define SERVER_IP "127.0.0.1" // The IP address of the server
#define REQUEST_SIZE 2048

#define GET 1
#define POST 2
#define DELETE 3

char *request_types[] = {"GET", "POST", "DELETE"};

char *request_generator(int request_type, char *content, int contetn_size)
{
    char *request_data = (char *)malloc(sizeof(char) * REQUEST_SIZE);

    if (request_type == 2 && contetn_size == 0)
        return NULL;

    snprintf(request_data, REQUEST_SIZE, "Content-Type:custom-text\nRequest-Type:%s\nContent-Size:%d\nContent:%s\n",
             request_types[request_type - 1],
             contetn_size,
             content);

    return request_data;
}

int main()
{
    struct addrinfo hints, *res;
    int sockfd;
    char message[1024] = {0}, buffer[1024] = {0}, content[1024] = {0};
    char *request_data;
    int bytes_sent, bytes_recived;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &res) != 0)
    {
        perror("getaddrinfo");
        return 1;
    }

    // Create socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
    {
        perror("Socket Error");
        return 1;
    }

    // Connect to the server
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("Connect Error");
        return 1;
    }

    printf("Connected to server !\n");

    // Main loop for sending and receiving data
    while (1)
    {
        bytes_recived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_recived == -1)
        {
            perror("Recived");
            break;
        }

        buffer[bytes_recived] = '\0';
        printf("%s\n", buffer);

        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        if (atoi(message) == 2)
        {
            printf("Enter post content \n");
            fgets(content, sizeof(content), stdin);
            content[strcspn(content, "\n")] = '\0';
        }

        request_data = request_generator(atoi(message), content, strlen(content));
        // Send the message to the server
        bytes_sent = send(sockfd, request_data, strlen(request_data), 0);

        if (bytes_sent == -1)
        {
            perror("Sent");
            break;
        }
        free(request_data);
    }

    freeaddrinfo(res);
    close(sockfd);
    return 0;
}