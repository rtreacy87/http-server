#ifndef ROUTER_H
#define ROUTER_H

#include "http.h"
//
// Define the route structure
typedef struct {
    const char* path;
    void (*handler)(http_request_t*, http_response_t*);
} route_t;

#define MAX_ROUTES 50

// Expose routes array and count as extern
extern route_t routes[MAX_ROUTES];
extern int route_count;


void register_route(const char* path, void (*handler)(http_request_t*, http_response_t*));
void handle_route(http_request_t* request, http_response_t* response);

void handle_home_page(http_request_t* request, http_response_t* response);
void handle_hello_page(http_request_t* request, http_response_t* response);
void handle_not_found(http_response_t* response);

#endif
