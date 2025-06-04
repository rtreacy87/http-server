# Phase 4: Dynamic Content Generation

## What We're Building

In this phase, we'll create a flexible routing system that allows us to:
- Register custom handlers for different URL patterns
- Handle POST requests with form data
- Generate dynamic HTML content
- Create a simple API with JSON responses
- Handle URL parameters and query strings

## Understanding Dynamic Content

Static files are the same every time, but dynamic content changes based on:
- Current time/date
- User input (forms, URL parameters)
- Database queries
- External API calls
- Server state

## Step 1: Create a Routing System

Create `include/router.h`:

```c
#ifndef ROUTER_H
#define ROUTER_H

#include "http.h"

#define MAX_ROUTES 50
#define MAX_ROUTE_PATH 256

typedef struct {
    char method[16];
    char path[MAX_ROUTE_PATH];
    int (*handler)(http_request_t* request, http_response_t* response);
} route_t;

typedef struct {
    route_t routes[MAX_ROUTES];
    int route_count;
} router_t;

// Router functions
void init_router(router_t* router);
int add_route(router_t* router, const char* method, const char* path, 
              int (*handler)(http_request_t*, http_response_t*));
int route_request(router_t* router, http_request_t* request, http_response_t* response);

// Utility functions
char* get_header_value(http_request_t* request, const char* header_name);
char* url_decode(const char* encoded);
void parse_query_string(const char* query, char params[][2][256], int* param_count);

#endif
```

## Step 2: Implement the Router

Create `src/router.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/router.h"

void init_router(router_t* router) {
    router->route_count = 0;
    memset(router->routes, 0, sizeof(router->routes));
}

int add_route(router_t* router, const char* method, const char* path,
              int (*handler)(http_request_t*, http_response_t*)) {
    if (router->route_count >= MAX_ROUTES) {
        return -1; // Router full
    }
    
    route_t* route = &router->routes[router->route_count];
    strncpy(route->method, method, sizeof(route->method) - 1);
    strncpy(route->path, path, sizeof(route->path) - 1);
    route->handler = handler;
    
    router->route_count++;
    return 0;
}

int route_request(router_t* router, http_request_t* request, http_response_t* response) {
    for (int i = 0; i < router->route_count; i++) {
        route_t* route = &router->routes[i];
        
        if (strcmp(route->method, request->method) == 0 &&
            strcmp(route->path, request->uri) == 0) {
            return route->handler(request, response);
        }
    }
    return -1; // No route found
}

char* get_header_value(http_request_t* request, const char* header_name) {
    for (int i = 0; i < request->header_count; i++) {
        if (strcasecmp(request->headers[i][0], header_name) == 0) {
            return request->headers[i][1];
        }
    }
    return NULL;
}

char* url_decode(const char* encoded) {
    size_t len = strlen(encoded);
    char* decoded = malloc(len + 1);
    if (!decoded) return NULL;
    
    size_t i = 0, j = 0;
    while (i < len) {
        if (encoded[i] == '%' && i + 2 < len) {
            // Decode %XX
            char hex[3] = {encoded[i+1], encoded[i+2], '\0'};
            decoded[j] = (char)strtol(hex, NULL, 16);
            i += 3;
        } else if (encoded[i] == '+') {
            // Convert + to space
            decoded[j] = ' ';
            i++;
        } else {
            decoded[j] = encoded[i];
            i++;
        }
        j++;
    }
    decoded[j] = '\0';
    return decoded;
}

void parse_query_string(const char* query, char params[][2][256], int* param_count) {
    *param_count = 0;
    if (!query) return;
    
    char* query_copy = strdup(query);
    char* pair = strtok(query_copy, "&");
    
    while (pair && *param_count < MAX_HEADERS) {
        char* equals = strchr(pair, '=');
        if (equals) {
            *equals = '\0';
            char* key = url_decode(pair);
            char* value = url_decode(equals + 1);
            
            if (key && value) {
                strncpy(params[*param_count][0], key, 255);
                strncpy(params[*param_count][1], value, 255);
                (*param_count)++;
            }
            
            free(key);
            free(value);
        }
        pair = strtok(NULL, "&");
    }
    
    free(query_copy);
}
```

## Step 3: Create Route Handlers

Create `src/handlers.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../include/router.h"

// Helper function to set response
void set_response(http_response_t* response, int status, const char* content_type, const char* body) {
    response->status_code = status;
    if (response->body) free(response->body);
    response->body = strdup(body);
    response->body_length = strlen(body);
    
    strcpy(response->headers[0][0], "Content-Type");
    strcpy(response->headers[0][1], content_type);
    response->header_count = 1;
}

// API endpoint: GET /api/time
int handle_api_time(http_request_t* request, http_response_t* response) {
    (void)request; // Unused parameter
    
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    char json_response[512];
    snprintf(json_response, sizeof(json_response),
        "{\n"
        "  \"timestamp\": %ld,\n"
        "  \"formatted\": \"%04d-%02d-%02d %02d:%02d:%02d\",\n"
        "  \"timezone\": \"Local\"\n"
        "}",
        now,
        tm_info->tm_year + 1900,
        tm_info->tm_mon + 1,
        tm_info->tm_mday,
        tm_info->tm_hour,
        tm_info->tm_min,
        tm_info->tm_sec
    );
    
    set_response(response, 200, "application/json", json_response);
    return 0;
}

// API endpoint: GET /api/echo?message=hello
int handle_api_echo(http_request_t* request, http_response_t* response) {
    // Parse query string
    char* query_start = strchr(request->uri, '?');
    char params[MAX_HEADERS][2][256];
    int param_count = 0;
    
    if (query_start) {
        parse_query_string(query_start + 1, params, &param_count);
    }
    
    // Find message parameter
    char* message = "No message provided";
    for (int i = 0; i < param_count; i++) {
        if (strcmp(params[i][0], "message") == 0) {
            message = params[i][1];
            break;
        }
    }
    
    char json_response[1024];
    snprintf(json_response, sizeof(json_response),
        "{\n"
        "  \"echo\": \"%s\",\n"
        "  \"length\": %zu\n"
        "}",
        message, strlen(message)
    );
    
    set_response(response, 200, "application/json", json_response);
    return 0;
}

// Form handler: POST /api/contact
int handle_contact_form(http_request_t* request, http_response_t* response) {
    if (!request->body) {
        set_response(response, 400, "application/json", "{\"error\": \"No form data\"}");
        return 0;
    }
    
    // Parse form data (application/x-www-form-urlencoded)
    char params[MAX_HEADERS][2][256];
    int param_count = 0;
    parse_query_string(request->body, params, &param_count);
    
    // Extract form fields
    char* name = "Unknown";
    char* email = "Not provided";
    char* message = "No message";
    
    for (int i = 0; i < param_count; i++) {
        if (strcmp(params[i][0], "name") == 0) {
            name = params[i][1];
        } else if (strcmp(params[i][0], "email") == 0) {
            email = params[i][1];
        } else if (strcmp(params[i][0], "message") == 0) {
            message = params[i][1];
        }
    }
    
    // Log the form submission (in real app, save to database)
    printf("Contact form submitted:\n");
    printf("  Name: %s\n", name);
    printf("  Email: %s\n", email);
    printf("  Message: %s\n", message);
    
    char json_response[1024];
    snprintf(json_response, sizeof(json_response),
        "{\n"
        "  \"status\": \"success\",\n"
        "  \"message\": \"Thank you, %s! We received your message.\"\n"
        "}",
        name
    );
    
    set_response(response, 200, "application/json", json_response);
    return 0;
}

// Dynamic HTML page: GET /status
int handle_status_page(http_request_t* request, http_response_t* response) {
    (void)request; // Unused parameter
    
    time_t now = time(NULL);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; // Remove newline
    
    char html_response[2048];
    snprintf(html_response, sizeof(html_response),
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>Server Status</title>\n"
        "    <style>\n"
        "        body { font-family: Arial, sans-serif; margin: 40px; }\n"
        "        .status { background: #e8f5e8; padding: 20px; border-radius: 5px; }\n"
        "        .info { margin: 10px 0; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>Server Status</h1>\n"
        "    <div class=\"status\">\n"
        "        <div class=\"info\"><strong>Status:</strong> Running</div>\n"
        "        <div class=\"info\"><strong>Current Time:</strong> %s</div>\n"
        "        <div class=\"info\"><strong>Process ID:</strong> %d</div>\n"
        "        <div class=\"info\"><strong>Server:</strong> Custom HTTP Server v1.0</div>\n"
        "    </div>\n"
        "    <p><a href=\"/\">← Back to Home</a></p>\n"
        "</body>\n"
        "</html>",
        time_str, getpid()
    );
    
    set_response(response, 200, "text/html", html_response);
    return 0;
}

// Contact form page: GET /contact
int handle_contact_page(http_request_t* request, http_response_t* response) {
    (void)request; // Unused parameter
    
    const char* html_response = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>Contact Us</title>\n"
        "    <style>\n"
        "        body { font-family: Arial, sans-serif; margin: 40px; max-width: 600px; }\n"
        "        form { background: #f9f9f9; padding: 20px; border-radius: 5px; }\n"
        "        label { display: block; margin: 10px 0 5px; font-weight: bold; }\n"
        "        input, textarea { width: 100%; padding: 8px; margin-bottom: 10px; border: 1px solid #ddd; border-radius: 3px; }\n"
        "        button { background: #007acc; color: white; padding: 10px 20px; border: none; border-radius: 3px; cursor: pointer; }\n"
        "        button:hover { background: #005a99; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>Contact Us</h1>\n"
        "    <form action=\"/api/contact\" method=\"POST\">\n"
        "        <label for=\"name\">Name:</label>\n"
        "        <input type=\"text\" id=\"name\" name=\"name\" required>\n"
        "        \n"
        "        <label for=\"email\">Email:</label>\n"
        "        <input type=\"email\" id=\"email\" name=\"email\" required>\n"
        "        \n"
        "        <label for=\"message\">Message:</label>\n"
        "        <textarea id=\"message\" name=\"message\" rows=\"5\" required></textarea>\n"
        "        \n"
        "        <button type=\"submit\">Send Message</button>\n"
        "    </form>\n"
        "    <p><a href=\"/\">← Back to Home</a></p>\n"
        "</body>\n"
        "</html>";
    
    set_response(response, 200, "text/html", html_response);
    return 0;
}
```

## Step 4: Update Main Server

Update `src/server.c` to use the router:

```c
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

// Declare handler functions
int handle_api_time(http_request_t* request, http_response_t* response);
int handle_api_echo(http_request_t* request, http_response_t* response);
int handle_contact_form(http_request_t* request, http_response_t* response);
int handle_status_page(http_request_t* request, http_response_t* response);
int handle_contact_page(http_request_t* request, http_response_t* response);

router_t global_router;

void setup_routes() {
    init_router(&global_router);
    
    // API routes
    add_route(&global_router, "GET", "/api/time", handle_api_time);
    add_route(&global_router, "GET", "/api/echo", handle_api_echo);
    add_route(&global_router, "POST", "/api/contact", handle_contact_form);
    
    // Page routes
    add_route(&global_router, "GET", "/status", handle_status_page);
    add_route(&global_router, "GET", "/contact", handle_contact_page);
}

void handle_request(int client_fd, const char* raw_request) {
    http_request_t request;
    http_response_t response;
    
    init_http_response(&response);
    
    if (parse_http_request(raw_request, &request) == 0) {
        printf("Method: %s, URI: %s\n", request.method, request.uri);
        
        // Extract path without query string for routing
        char path[MAX_URI_SIZE];
        strncpy(path, request.uri, sizeof(path) - 1);
        char* query_start = strchr(path, '?');
        if (query_start) {
            *query_start = '\0';
        }
        
        // Try dynamic routes first
        http_request_t route_request = request;
        strncpy(route_request.uri, path, sizeof(route_request.uri) - 1);
        
        if (route_request(global_router, &route_request, &response) == 0) {
            printf("Handled by dynamic route: %s\n", path);
        } else if (strcmp(request.method, "GET") == 0) {
            // Try static files
            if (serve_static_file(request.uri, &response) == 0) {
                printf("Served static file: %s\n", request.uri);
            } else {
                // 404 Not Found
                response.status_code = 404;
                response.body = strdup("<!DOCTYPE html><html><body><h1>404 - Page Not Found</h1><p><a href=\"/\">Go Home</a></p></body></html>");
                response.body_length = strlen(response.body);
                strcpy(response.headers[0][0], "Content-Type");
                strcpy(response.headers[0][1], "text/html");
                response.header_count = 1;
            }
        } else {
            // Method not allowed
            response.status_code = 405;
            response.body = strdup("Method not allowed");
            response.body_length = strlen(response.body);
            strcpy(response.headers[0][0], "Content-Type");
            strcpy(response.headers[0][1], "text/plain");
            response.header_count = 1;
        }
    } else {
        // Bad request
        response.status_code = 400;
        response.body = strdup("Bad request");
        response.body_length = strlen(response.body);
        strcpy(response.headers[0][0], "Content-Type");
        strcpy(response.headers[0][1], "text/plain");
        response.header_count = 1;
    }
    
    send_http_response(client_fd, &response);
    
    free_http_request(&request);
    free_http_response(&response);
}

int main() {
    setup_routes();
    
    // ... rest of socket setup code same as before
    
    return 0;
}
```

## Step 5: Update Build System

Update `Makefile`:

```makefile
CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRCDIR=src
INCDIR=include
SOURCES=$(SRCDIR)/server.c $(SRCDIR)/http_parser.c $(SRCDIR)/file_server.c $(SRCDIR)/router.c $(SRCDIR)/handlers.c
TARGET=server

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
```

## Step 6: Test Dynamic Content

### Compile and Run
```bash
make clean
make
./server
```

### Test API Endpoints
```bash
# Get current time
curl http://localhost:8080/api/time

# Echo with query parameter
curl "http://localhost:8080/api/echo?message=Hello%20World"

# Test contact form
curl -X POST -d "name=John&email=john@example.com&message=Hello!" \
     http://localhost:8080/api/contact
```

### Test Dynamic Pages
Visit in browser:
- `http://localhost:8080/status` - Server status page
- `http://localhost:8080/contact` - Contact form

## Understanding the Code

### Router System
- **Routes**: Map URL patterns to handler functions
- **Handlers**: Functions that process requests and generate responses
- **Flexible**: Easy to add new endpoints

### Query String Parsing
```c
// URL: /api/echo?message=hello&name=world
// Becomes: params[0] = {"message", "hello"}, params[1] = {"name", "world"}
```

### Form Data Handling
- POST requests with `application/x-www-form-urlencoded`
- Same format as query strings but in request body
- Automatically URL-decoded

## Next Steps

You now have a powerful HTTP server with:
- Static file serving
- Dynamic routing system
- API endpoints with JSON responses
- Form handling
- Query parameter parsing
- Dynamic HTML generation

In the next phase, we'll add advanced features like persistent connections, better error handling, and basic security measures.

---

**Previous:** [← Static File Serving](04-static-files.md) | **Next:** [Phase 5: Advanced Features →](06-advanced-features.md)
