#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/http.h"
#include "../include/router.h"

#define PORT 8080
#define BUFFER_SIZE 4096

void handle_method_not_allowed(http_response_t* response) {
    response->status_code = 405; // Method not allowed
    response->body = strdup("Method not allowed");
    response->body_length = strlen(response->body);
    strcpy(response->headers[0][0], "Content-Type");
    strcpy(response->headers[0][1], "text/plain");
    response->header_count = 1;
}

void handle_bad_request(http_response_t* response) {
    response->status_code = 400; // Bad request
    response->body = strdup("Bad request");
    response->body_length = strlen(response->body);
    strcpy(response->headers[0][0], "Content-Type");
    strcpy(response->headers[0][1], "text/plain");
    response->header_count = 1;
}

void handle_good_request(http_request_t* request, http_response_t* response) {
    printf("Method: %s\n", request->method);
    printf("URI: %s\n", request->uri);
    printf("Version: %s\n", request->version);
    printf("Headers: %d\n", request->header_count);
    
    // Simple routing
    if (strcmp(request->method, "GET") == 0) {
        printf("Calling handle_route for GET request\n");
        handle_route(request, response);
        printf("Returned from handle_route\n");

    } else {
        handle_method_not_allowed(response);
    }
}

void handle_request(int client_fd, const char* raw_request) {
    http_request_t request;
    http_response_t response;
    init_http_response(&response);
    if (parse_http_request(raw_request, &request) == 0) {
        handle_good_request(&request, &response);
    } else {
        handle_bad_request(&response);
    }
    send_http_response(client_fd, &response);
    free_http_request(&request);
    free_http_response(&response);
}

void setup_routes() {
    // Clear existing routes
    route_count = 0;
    
    // Register routes
    register_route("/", handle_home_page);
    register_route("/hello", handle_hello_page);
    
    printf("Routes registered: %d\n", route_count);
    for (int i = 0; i < route_count; i++) {
        printf("  Route %d: %s\n", i, routes[i].path);
    }
}



int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    printf("Starting server on port %d...\n", PORT);

    printf("Raw request first 100 chars: %.100s\n", buffer);   
    // Step 1: Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Step 2: Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Step 3: Bind socket to address
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }
    
    // Step 4: Listen for connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
    
    printf("Server listening on port %d\n", PORT);
    setup_routes();
    while (1) {
        printf("Waiting for connection...\n");
        
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("Client connected from %s\n", inet_ntoa(client_addr.sin_addr));
        
        // Read HTTP request
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Raw request:\n%s\n", buffer);
            
            handle_request(client_fd, buffer);
        }
        
        close(client_fd);
        printf("Client disconnected\n");
    }
    
    return 0;
}

