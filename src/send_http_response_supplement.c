#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/http.h"

void update_headers(int client_fd, http_response_t* response) {
    // Headers
    for (int i = 0; i < response->header_count; i++) {
        char header_line[1024];
        snprintf(header_line, sizeof(header_line), "%s: %s\r\n",
                response->headers[i][0], response->headers[i][1]);
        write(client_fd, header_line, strlen(header_line));
    }
}

void update_content_length(int client_fd, http_response_t* response) {
    // Content-Length header if we have a body
    if (response->body && response->body_length > 0) {
        char content_length[64];
        snprintf(content_length, sizeof(content_length), "Content-Length: %zu\r\n", 
                response->body_length);
        write(client_fd, content_length, strlen(content_length));
    }
}

void add_headers(int client_fd, http_response_t* response){
   update_headers(client_fd, response);
   update_content_length(client_fd, response);
   // End of headers
   write(client_fd, "\r\n", 2);
}

void update_status_line(int client_fd, http_response_t* response) {
    // Update status line based on response code
    char status_line[256];
    const char* status_text = "OK";
    if (response->status_code == 404) status_text = "Not Found";
    else if (response->status_code == 500) status_text = "Internal Server Error";
    
    snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
             response->status_code, status_text);
    write(client_fd, status_line, strlen(status_line));
}



void write_body(int client_fd, http_response_t* response) {
    // Body
    if (response->body && response->body_length > 0) {
        write(client_fd, response->body, response->body_length);
    }
}

