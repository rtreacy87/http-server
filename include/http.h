
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#define MAX_HEADERS 50
#define MAX_HEADER_SIZE 256
#define MAX_URI_SIZE 1024
#define MAX_METHOD_SIZE 16
#define MAX_VERSION_SIZE 16

typedef struct {
    char method[MAX_METHOD_SIZE];
    char uri[MAX_URI_SIZE];
    char version[MAX_VERSION_SIZE];
    char headers[MAX_HEADERS][2][MAX_HEADER_SIZE];  // [index][key/value][string]
    int header_count;
    char* body;
    size_t body_length;
} http_request_t;

typedef struct {
    int status_code;
    char headers[MAX_HEADERS][2][MAX_HEADER_SIZE];
    int header_count;
    char* body;
    size_t body_length;
} http_response_t;

// Function declarations
int parse_http_request(const char* raw_request, http_request_t* request);
void init_http_request(http_request_t* request);
void free_http_request(http_request_t* request);
void init_http_response(http_response_t* response);
void free_http_response(http_response_t* response);
int send_http_response(int client_fd, http_response_t* response);

#endif

