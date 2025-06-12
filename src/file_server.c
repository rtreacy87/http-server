#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../include/http.h"
#include "../include/build_file_path_supplement.h"
#include "../include/serve_static_file_supplement.h"

#define DOCUMENT_ROOT "./static"

// MIME type mapping
typedef struct {
    const char* extension;
    const char* mime_type;
} mime_mapping_t;

static mime_mapping_t mime_types[] = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {".txt", "text/plain"},
    {".pdf", "application/pdf"},
    {NULL, "application/octet-stream"} // Default
};

const char* get_mime_type(const char* filename) {
    const char* extension = strrchr(filename, '.');
    if (!extension) {
        return "application/octet-stream";
    }
    
    for (int i = 0; mime_types[i].extension; i++) {
        if (strcasecmp(extension, mime_types[i].extension) == 0) {
            return mime_types[i].mime_type;
        }
    }
    
    return "application/octet-stream";
}

int is_safe_path(const char* path) {
    // Prevent directory traversal attacks
    if (strstr(path, "..") != NULL) {
        return 0;
    }
    if (strstr(path, "//") != NULL) {
        return 0;
    }
    return 1;
}


char* build_file_path(const char* uri) {
    // Security check
    if (!is_safe_path(uri)) {
        return NULL;
    }
    // Get the file path, possibly applying default file rules
    const char* file_path = get_default_file_path(uri);
    // Construct the full path by combining document root and file path
    return construct_full_path(DOCUMENT_ROOT, file_path);
}

// Set response headers for static file
void set_static_file_headers(http_response_t* response, const char* file_path) {
    // Content-Type header
    const char* mime_type = get_mime_type(file_path);
    strcpy(response->headers[response->header_count][0], "Content-Type");
    strcpy(response->headers[response->header_count][1], mime_type);
    response->header_count++;
    
    // Cache-Control header for static files
    strcpy(response->headers[response->header_count][0], "Cache-Control");
    strcpy(response->headers[response->header_count][1], "public, max-age=3600");
    response->header_count++;
}
void serve_static_file_handler(http_request_t* request, http_response_t* response) {
    const char* uri = request->uri;
    char* file_path = build_file_path(uri);
    if (!file_path) {
        // Invalid path - set 404 response
        response->status_code = 404;
        response->body = strdup("File not found");
        response->body_length = strlen(response->body);
        strcpy(response->headers[0][0], "Content-Type");
        strcpy(response->headers[0][1], "text/plain");
        response->header_count = 1;
        return;
    }
    
    // Validate the file
    struct stat file_stat;
    if (validate_file(file_path, &file_stat) != 0) {
        free(file_path);
        // File validation failed - set 404 response
        response->status_code = 404;
        response->body = strdup("File not found");
        response->body_length = strlen(response->body);
        strcpy(response->headers[0][0], "Content-Type");
        strcpy(response->headers[0][1], "text/plain");
        response->header_count = 1;
        return;
    }
    
    // Read the file content
    size_t bytes_read;
    response->body = read_file_content(file_path, file_stat.st_size, &bytes_read);
    if (!response->body) {
        free(file_path);
        // Failed to read file - set 500 response
        response->status_code = 500;
        response->body = strdup("Internal server error");
        response->body_length = strlen(response->body);
        strcpy(response->headers[0][0], "Content-Type");
        strcpy(response->headers[0][1], "text/plain");
        response->header_count = 1;
        return;
    }
    response->body_length = bytes_read;
    
    // Set response status and headers
    response->status_code = 200;
    set_static_file_headers(response, file_path);
    
    free(file_path);
}
// Main function to serve a static file
int serve_static_file(const char* uri, http_response_t* response) {
    // Create a dummy request with just the URI
    http_request_t dummy_request;
    memset(&dummy_request, 0, sizeof(http_request_t));
    
    // Copy the URI into the request's URI field instead of assigning
    strncpy(dummy_request.uri, uri, MAX_URI_SIZE - 1);
    dummy_request.uri[MAX_URI_SIZE - 1] = '\0'; // Ensure null termination
    
    // Save the current status code to check if it changes
    //int original_status = response->status_code;
    
    // Call the handler version
    serve_static_file_handler(&dummy_request, response);
    
    // Return success (0) if status is 200, otherwise failure (-1)
    return (response->status_code == 200) ? 0 : -1;
}
