#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/http.h"


char* find_header_end(const char* raw_request) {
    const char* header_end = strstr(raw_request, "\r\n\r\n");
    if (!header_end) {
        header_end = strstr(raw_request, "\n\n");
    }
    return (char*)header_end;
}

char* find_request_line_end(const char* raw_request) {
    return strchr(raw_request, '\n');
}

char* copy_line(const char* start, const char* end) {
    size_t length = end - start;
    char* line = malloc(length + 1);
    if (!line) return NULL;
    
    strncpy(line, start, length);
    line[length] = '\0';
    
    // Remove \r if present
    if (length > 0 && line[length - 1] == '\r') {
        line[length - 1] = '\0';
    }
    
    return line;
}

int parse_header(const char* header_line, http_request_t* request) {
    // Check if we've reached the maximum number of headers
    if (request->header_count >= MAX_HEADERS) {
        return -1;
    }
    
    // Parse header key: value
    char* colon = strchr(header_line, ':');
    if (!colon) {
        return -1;  // Invalid header format
    }
    
    // Split the header into key and value
    *colon = '\0';  // Temporarily modify the string to split it
    char* key = (char*)header_line;
    char* value = colon + 1;
    
    // Skip leading whitespace in value
    while (*value == ' ' || *value == '\t') value++;
    
    // Copy key and value to the request structure
    strncpy(request->headers[request->header_count][0], key, MAX_HEADER_SIZE - 1);
    strncpy(request->headers[request->header_count][1], value, MAX_HEADER_SIZE - 1);
    request->header_count++;
    
    return 0;
}
