#include "requests_handlers.h"

char *handle_get_request()
{
    int fd = open(DB_NAME, O_RDONLY | O_CREAT, 0644);
    if (fd == -1)
    {
        perror("Failed to open file");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        perror("Failed to get file stats");
        close(fd);
        return NULL;
    }

    char *buffer = (char *)malloc(sizeof(char) * (st.st_size + 1));
    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        close(fd);
        return NULL;
    }

    int bytes_read = read(fd, buffer, st.st_size);
    if (bytes_read == -1)
    {
        perror("Failed to read file");
        free(buffer);
        close(fd);
        return NULL;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the buffer
    close(fd);
    return buffer;
}

int handle_post_request(char *data)
{
    int fd = open(DB_NAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        perror("Failed to open file");
        return 0;
    }

    int bytes_written = write(fd, data, strlen(data));
    if (bytes_written == -1)
    {
        perror("Failed to write to file");
        close(fd);
        return 0;
    }

    close(fd);
    return 1;
}

int handle_delete_request()
{
    if (remove(DB_NAME) == 0)
    {
        return 1;
    }

    perror("Failed to delete file");
    return 0;
}