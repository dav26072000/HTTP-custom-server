#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
// Custom Functions
#include "requests_handlers.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

#define PORT "8080"
#define BACKLOG 10

// Action Status for unauthenticated users
#define NO_ACTION 0
#define GET 1
#define POST 2
#define DELETE 3

void safe_strcat(char *result, const char *current, size_t result_size)
{
    if (strlen(result) + strlen(current) < result_size)
    {
        strcat(result, current);
    }
}

void print_unauth_menu(int client_fd, const char *message, int status)
{
    char menu[1024]; // A buffer to store the final message, adjust size if needed

    // Add the custom message first, followed by the menu
    if (status)
    {
        snprintf(menu, sizeof(menu), "Status = %d\nServer Support this type of requests \n"
                                     "1 : GET \n"
                                     "2 : POST \n"
                                     "3 : DELETE \n",
                 status);
    }
    else if (message && strlen(message) > 0)
    {
        snprintf(menu, sizeof(menu), "%s\nServer Support this type of requests \n"
                                     "1 : GET \n"
                                     "2 : POST \n"
                                     "3 : DELETE \n",
                 message);
    }
    else
    {
        snprintf(menu, sizeof(menu), "Server Support this type of requests \n"
                                     "1 : GET \n"
                                     "2 : POST \n"
                                     "3 : DELETE \n");
    }

    // Send the constructed message to the client
    send(client_fd, menu, strlen(menu), 0);
}

int get_action_code(char *head_value)
{
    if (strcmp(head_value, "GET") == 0)
    {
        return 1;
    }
    else if (strcmp(head_value, "POST") == 0)
    {
        return 2;
    }
    else if (strcmp(head_value, "DELETE") == 0)
    {
        return 3;
    }
    else
    {
        return 0;
    }
}

int is_response_line_valid(char *head_name, char *head_value, int *action_code, char *content)
{
    if (strcmp(head_name, "Content-Type") == 0 && strcmp(head_value, "custom-text") == 0)
        return 1;

    if ((strcmp(head_name, "Request-Type") == 0) && (strcmp(head_value, "GET") == 0 || strcmp(head_value, "POST") == 0 || strcmp(head_value, "DELETE") == 0))
    {
        *action_code = get_action_code(head_value);
        return 1;
    }
    if (strcmp(head_name, "Content") == 0)
    {
        strcpy(content, head_value);
        return 1;
    }
    if (strcmp(head_name, "Content-Size") == 0)
    {
        return 1;
    }

    return 0;
}

int response_validation(char *response, int client_fd, int *action_code, char *content)
{
    char *resp_copy = (char *)malloc(sizeof(char) * 1024);
    strcpy(resp_copy, response);

    char head_name[1024] = {0};
    char head_value[1024] = {0};
    int is_before_dot = 1;
    for (int i = 0, k = 0, j = 0; i < strlen(resp_copy); ++i)
    {
        if (is_before_dot)
        {
            if (resp_copy[i] == ':')
            {
                head_name[k] = '\0';
                is_before_dot = 0;
                k = 0;
                continue;
            }
            head_name[k++] = resp_copy[i];
        }
        else
        {
            if (resp_copy[i] == '\n' || resp_copy[i] == '\0')
            {
                head_value[j] = '\0';
                if (!is_response_line_valid(head_name, head_value, action_code, content))
                {
                    print_unauth_menu(client_fd, "", 401);
                    break;
                }

                is_before_dot = 1;
                j = 0;
                if (resp_copy[i] == '\0')
                    break;

                continue;
            }
            head_value[j++] = resp_copy[i];
        }
    }

    free(resp_copy);
}

void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);
    print_unauth_menu(client_fd, "", 0);
    int action_status = NO_ACTION;

    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_read] = '\0';
        printf("Recive : %s \n", buffer);
        int action_code = 0;
        char *content = (char *)malloc(sizeof(char) * 1024);
        response_validation(buffer, client_fd, &action_code, content);

        if (action_code == GET)
        {
            char *data = handle_get_request();
            if (data)
            {
                print_unauth_menu(client_fd, data, 0);
                free(data);
                continue;
            }
            print_unauth_menu(client_fd, "", 400);
        }
        else if (action_code == POST)
        {
            printf("%s\n", content);
            if (handle_post_request(content))
            {
                print_unauth_menu(client_fd, "", 100);
                continue;
            }
            print_unauth_menu(client_fd, "", 400);
        }
        else if (action_code == DELETE)
        {
            if (handle_delete_request())
            {
                print_unauth_menu(client_fd, "", 100);
                continue;
            }
            print_unauth_menu(client_fd, "", 400);
        }
    }

    if (bytes_read == -1)
    {
        perror("bytes_read");
    }

    close(client_fd);
    printf("Client disconected \n");

    return NULL;
}

void *handle_sockets(void *arg)
{
    struct addrinfo hints,
        *res;
    int server_fd;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    char client_ip[INET6_ADDRSTRLEN];
    pthread_t clientsThreads[BACKLOG];

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &res) != 0)
    {
        perror("getaddrinfo");
        return NULL;
    }

    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (server_fd == -1)
    {
        perror("socket");
        pthread_exit(NULL);
    }

    if (bind(server_fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind");
        pthread_exit(NULL);
    }
    freeaddrinfo(res);

    if (listen(server_fd, BACKLOG) == -1)
    {
        perror("listen");
        pthread_exit(NULL);
    }
    printf("Server is listening on port %s...", PORT);

    while (1)
    {
        addr_size = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));

        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
        if (*client_fd == -1)
        {
            perror("Accept Error");
            free(client_fd);
            continue;
        }

        inet_ntop(client_addr.ss_family, &((struct sockaddr_in *)&client_addr)->sin_addr, client_ip, sizeof(client_ip));
        printf("New connection from %s\n", client_ip);

        // Create thread for each connection
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, client_fd) != 0)
        {
            perror("pthread_create");
            free(client_fd);
            continue;
        }

        pthread_detach(client_thread);
    }

    close(server_fd);
}

int main()
{
    // Ignore the SIGPIPE signal
    signal(SIGPIPE, SIG_IGN);
    // Create thread for each connection
    pthread_t socket_thread;
    if (pthread_create(&socket_thread, NULL, handle_sockets, NULL) != 0)
    {
        perror("pthread_create");
        return 1;
    }

    pthread_join(socket_thread, NULL);

    // pthread_detach(socket_thread);
    return 0;
}