#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/http.h"

typedef struct {
    const char* path;
    void (*handler)(http_request_t*, http_response_t*);
} route_t;

#define MAX_ROUTES 50
route_t routes[MAX_ROUTES];
int route_count = 0;

void handle_home_page(http_request_t* request, http_response_t* response) {
    (void)request; // Unused parameter
    response->status_code = 200;
    response->body = strdup("<html><body><h1>Welcome to our HTTP Server!</h1></body></html>");
    response->body_length = strlen(response->body);
    strcpy(response->headers[0][0], "Content-Type");
    strcpy(response->headers[0][1], "text/html");
    response->header_count = 1;
}

void handle_hello_page(http_request_t* request, http_response_t* response) {
    (void)request; // Unused parameter
    response->status_code = 200;
    response->body = strdup("Hello, World!");
    response->body_length = strlen(response->body);
    strcpy(response->headers[0][0], "Content-Type");
    strcpy(response->headers[0][1], "text/plain");
    response->header_count = 1;
}

void handle_not_found(http_request_t* request, http_response_t* response) {
    (void)request; // Unused parameter
    response->status_code = 404;
    response->body = strdup("Page not found");
    response->body_length = strlen(response->body);
    strcpy(response->headers[0][0], "Content-Type");
    strcpy(response->headers[0][1], "text/plain");
    response->header_count = 1;
}

void register_route(const char* path, void (*handler)(http_request_t*, http_response_t*)) {
    if (route_count < MAX_ROUTES) {
        routes[route_count].path = path;
        routes[route_count].handler = handler;
        route_count++;
    }
}

void handle_route(http_request_t* request, http_response_t* response) {
    // Try to find a matching route 
    for (int i = 0; i < route_count; i++) {
        if (strcmp(request->uri, routes[i].path) == 0) {
            // Call the registered handler for this route
            routes[i].handler(request, response);
            return;
        }
    }
    
    // No route found, handle 404
    handle_not_found(request, response);
}


