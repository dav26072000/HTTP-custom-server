#ifndef REQUESTS_HANDLER
#define REQUESTS_HANDLER

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define DB_NAME "db.txt"

char *handle_get_request();
int handle_post_request(char *data);
int handle_delete_request();

#endif