#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/http.h"
#include "../include/parse_http_request_supplement.h"
#include "../include/send_http_response_supplement.h"


void init_http_request(http_request_t* request) {
    memset(request, 0, sizeof(http_request_t));
    request->body = NULL;
    request->body_length = 0;
    request->header_count = 0;
}


void free_http_request(http_request_t* request) {
    if (request->body) {
        free(request->body);
        request->body = NULL;
    }
}


void init_http_response(http_response_t* response) {
    memset(response, 0, sizeof(http_response_t));
    response->body = NULL;
    response->body_length = 0;
    response->header_count = 0;
    response->status_code = 200;
}


void free_http_response(http_response_t* response) {
    if (response->body) {
        free(response->body);
        response->body = NULL;
    }
}



int parse_http_request(const char* raw_request, http_request_t* request) {
    init_http_request(request);
    
    // Find the end of headers (double CRLF)
    const char* header_end = find_header_end(raw_request);
    if (!header_end) return -1;
 
    // Parse request line
    char* line_end = find_request_line_end(raw_request);
    if (!line_end) return -1;
    
    // Copy request line for parsing
    char* request_line = copy_line(raw_request, line_end);
    if (!request_line) return -1;  // Memory allocation failed

    // Parse method, URI, version
    char* token = strtok(request_line, " ");
    if (!token) return -1;
    strncpy(request->method, token, MAX_METHOD_SIZE - 1);
    
    token = strtok(NULL, " ");
    if (!token) return -1;
    strncpy(request->uri, token, MAX_URI_SIZE - 1);
    
    token = strtok(NULL, " ");
    if (!token) return -1;
    strncpy(request->version, token, MAX_VERSION_SIZE - 1);
    
    // Parse headers
    const char* current = strchr(raw_request, '\n') + 1;
    while (current < header_end && request->header_count < MAX_HEADERS) {
        line_end = strchr(current, '\n');
        if (!line_end) break;
        
        size_t line_length = line_end - current;
        if (line_length <= 1) break; // Empty line
        
        char* header_line = copy_line(current, line_end);
        if (!header_line) return -1;
        
        int result = parse_header(header_line, request);
        free(header_line);
        
        if (result != 0) {
           // Option 1: Strict - Fail the entire request
           return -1;
        
           // Option 2: Lenient - Just continue to next header
           // continue;
           
           // Option 3: Hybrid - Count errors
           // request->error_count++;
           // if (request->error_count > MAX_ALLOWED_ERRORS) return -1;
              // Handle error or just continue to next header
        }
        
        current = line_end + 1;
    }
    
    return 0; // Success
}

int send_http_response(int client_fd, http_response_t* response) {
    update_status_line(client_fd, response); 
    add_headers(client_fd, response); 
    write_body(client_fd, response);    
    return 0;
}


