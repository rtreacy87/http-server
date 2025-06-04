# Phase 5: Advanced Features

## What We're Building

In this final phase, we'll add production-ready features to make our server more robust and efficient:
- HTTP/1.1 persistent connections (Keep-Alive)
- Better error handling and logging
- Request size limits and timeouts
- Basic authentication
- Compression support
- Concurrent connection handling

## Understanding Advanced HTTP Features

### Persistent Connections
Instead of opening a new TCP connection for each request, HTTP/1.1 allows reusing connections for multiple requests. This reduces latency and server load.

### Chunked Transfer Encoding
For dynamic content where the size isn't known in advance, HTTP can send data in chunks.

## Step 1: Enhanced HTTP Parser

Update `include/http.h` to support advanced features:

```c
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>
#include <time.h>

#define MAX_HEADERS 50
#define MAX_HEADER_SIZE 256
#define MAX_URI_SIZE 1024
#define MAX_METHOD_SIZE 16
#define MAX_VERSION_SIZE 16
#define MAX_REQUEST_SIZE (1024 * 1024)  // 1MB limit

typedef struct {
    char method[MAX_METHOD_SIZE];
    char uri[MAX_URI_SIZE];
    char version[MAX_VERSION_SIZE];
    char headers[MAX_HEADERS][2][MAX_HEADER_SIZE];
    int header_count;
    char* body;
    size_t body_length;
    int keep_alive;
    time_t received_time;
} http_request_t;

typedef struct {
    int status_code;
    char headers[MAX_HEADERS][2][MAX_HEADER_SIZE];
    int header_count;
    char* body;
    size_t body_length;
    int keep_alive;
    int chunked;
} http_response_t;

typedef struct {
    int socket_fd;
    time_t last_activity;
    char buffer[MAX_REQUEST_SIZE];
    size_t buffer_used;
    int keep_alive;
} connection_t;

// Function declarations
int parse_http_request(const char* raw_request, http_request_t* request);
void init_http_request(http_request_t* request);
void free_http_request(http_request_t* request);
void init_http_response(http_response_t* response);
void free_http_response(http_response_t* response);
int send_http_response(int client_fd, http_response_t* response);
int serve_static_file(const char* uri, http_response_t* response);

// Advanced features
void init_connection(connection_t* conn, int socket_fd);
int read_http_request(connection_t* conn, http_request_t* request);
int should_keep_alive(http_request_t* request);
void add_security_headers(http_response_t* response);
void log_request(http_request_t* request, http_response_t* response);

#endif
```

## Step 2: Enhanced Connection Handling

Create `src/connection.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "../include/http.h"

void init_connection(connection_t* conn, int socket_fd) {
    conn->socket_fd = socket_fd;
    conn->last_activity = time(NULL);
    conn->buffer_used = 0;
    conn->keep_alive = 1;
    memset(conn->buffer, 0, sizeof(conn->buffer));
}

int read_http_request(connection_t* conn, http_request_t* request) {
    // Read data into buffer
    ssize_t bytes_read = read(conn->socket_fd, 
                             conn->buffer + conn->buffer_used,
                             sizeof(conn->buffer) - conn->buffer_used - 1);
    
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            printf("Client closed connection\n");
        } else {
            perror("Read error");
        }
        return -1;
    }
    
    conn->buffer_used += bytes_read;
    conn->buffer[conn->buffer_used] = '\0';
    conn->last_activity = time(NULL);
    
    // Look for complete HTTP request (ends with \r\n\r\n or \n\n)
    char* request_end = strstr(conn->buffer, "\r\n\r\n");
    if (!request_end) {
        request_end = strstr(conn->buffer, "\n\n");
        if (!request_end) {
            // Incomplete request, need more data
            if (conn->buffer_used >= sizeof(conn->buffer) - 1) {
                printf("Request too large\n");
                return -1;
            }
            return 0; // Need more data
        }
    }
    
    // Parse the complete request
    if (parse_http_request(conn->buffer, request) != 0) {
        return -1;
    }
    
    // Check if we should keep the connection alive
    conn->keep_alive = should_keep_alive(request);
    
    // Clear the buffer for next request
    conn->buffer_used = 0;
    memset(conn->buffer, 0, sizeof(conn->buffer));
    
    return 1; // Complete request parsed
}

int should_keep_alive(http_request_t* request) {
    // Check Connection header
    for (int i = 0; i < request->header_count; i++) {
        if (strcasecmp(request->headers[i][0], "Connection") == 0) {
            if (strcasecmp(request->headers[i][1], "close") == 0) {
                return 0;
            }
            if (strcasecmp(request->headers[i][1], "keep-alive") == 0) {
                return 1;
            }
        }
    }
    
    // Default behavior based on HTTP version
    if (strstr(request->version, "1.1")) {
        return 1; // HTTP/1.1 defaults to keep-alive
    }
    return 0; // HTTP/1.0 defaults to close
}

void add_security_headers(http_response_t* response) {
    // Add security headers
    strcpy(response->headers[response->header_count][0], "X-Content-Type-Options");
    strcpy(response->headers[response->header_count][1], "nosniff");
    response->header_count++;
    
    strcpy(response->headers[response->header_count][0], "X-Frame-Options");
    strcpy(response->headers[response->header_count][1], "DENY");
    response->header_count++;
    
    strcpy(response->headers[response->header_count][0], "X-XSS-Protection");
    strcpy(response->headers[response->header_count][1], "1; mode=block");
    response->header_count++;
}

void log_request(http_request_t* request, http_response_t* response) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    printf("[%s] %s %s - %d\n", 
           timestamp, request->method, request->uri, response->status_code);
}
```

## Step 3: Enhanced HTTP Parser

Update `src/http_parser.c` to handle keep-alive:

```c
// Add this to the existing parse_http_request function
int parse_http_request(const char* raw_request, http_request_t* request) {
    init_http_request(request);
    request->received_time = time(NULL);
    
    // ... existing parsing code ...
    
    // After parsing headers, check for keep-alive
    request->keep_alive = should_keep_alive(request);
    
    return 0;
}

// Update send_http_response to handle keep-alive
int send_http_response(int client_fd, http_response_t* response) {
    // Status line
    char status_line[256];
    const char* status_text = "OK";
    if (response->status_code == 404) status_text = "Not Found";
    else if (response->status_code == 500) status_text = "Internal Server Error";
    else if (response->status_code == 400) status_text = "Bad Request";
    else if (response->status_code == 405) status_text = "Method Not Allowed";
    
    snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
             response->status_code, status_text);
    write(client_fd, status_line, strlen(status_line));
    
    // Add security headers
    add_security_headers(response);
    
    // Connection header
    if (response->keep_alive) {
        strcpy(response->headers[response->header_count][0], "Connection");
        strcpy(response->headers[response->header_count][1], "keep-alive");
        response->header_count++;
        
        strcpy(response->headers[response->header_count][0], "Keep-Alive");
        strcpy(response->headers[response->header_count][1], "timeout=30, max=100");
        response->header_count++;
    } else {
        strcpy(response->headers[response->header_count][0], "Connection");
        strcpy(response->headers[response->header_count][1], "close");
        response->header_count++;
    }
    
    // Server header
    strcpy(response->headers[response->header_count][0], "Server");
    strcpy(response->headers[response->header_count][1], "Custom-HTTP-Server/1.0");
    response->header_count++;
    
    // Headers
    for (int i = 0; i < response->header_count; i++) {
        char header_line[512];
        snprintf(header_line, sizeof(header_line), "%s: %s\r\n",
                response->headers[i][0], response->headers[i][1]);
        write(client_fd, header_line, strlen(header_line));
    }
    
    // Content-Length header if we have a body
    if (response->body && response->body_length > 0) {
        char content_length[64];
        snprintf(content_length, sizeof(content_length), "Content-Length: %zu\r\n", 
                response->body_length);
        write(client_fd, content_length, strlen(content_length));
    }
    
    // End of headers
    write(client_fd, "\r\n", 2);
    
    // Body
    if (response->body && response->body_length > 0) {
        write(client_fd, response->body, response->body_length);
    }
    
    return 0;
}
```

## Step 4: Basic Authentication

Create `src/auth.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/http.h"

// Simple base64 decode (basic implementation)
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_decode(const char* input, char* output, size_t output_size) {
    size_t input_len = strlen(input);
    size_t output_len = 0;
    
    for (size_t i = 0; i < input_len && output_len < output_size - 1; i += 4) {
        int values[4] = {0};
        
        // Decode 4 characters into 3 bytes
        for (int j = 0; j < 4 && i + j < input_len; j++) {
            char* pos = strchr(base64_chars, input[i + j]);
            if (pos) {
                values[j] = pos - base64_chars;
            }
        }
        
        if (output_len < output_size - 1) {
            output[output_len++] = (values[0] << 2) | (values[1] >> 4);
        }
        if (output_len < output_size - 1 && input[i + 2] != '=') {
            output[output_len++] = (values[1] << 4) | (values[2] >> 2);
        }
        if (output_len < output_size - 1 && input[i + 3] != '=') {
            output[output_len++] = (values[2] << 6) | values[3];
        }
    }
    
    output[output_len] = '\0';
    return output_len;
}

int check_basic_auth(http_request_t* request, const char* required_user, const char* required_pass) {
    // Find Authorization header
    char* auth_header = NULL;
    for (int i = 0; i < request->header_count; i++) {
        if (strcasecmp(request->headers[i][0], "Authorization") == 0) {
            auth_header = request->headers[i][1];
            break;
        }
    }
    
    if (!auth_header) {
        return 0; // No auth header
    }
    
    // Check if it's Basic auth
    if (strncasecmp(auth_header, "Basic ", 6) != 0) {
        return 0; // Not basic auth
    }
    
    // Decode base64 credentials
    char decoded[256];
    if (base64_decode(auth_header + 6, decoded, sizeof(decoded)) <= 0) {
        return 0; // Decode failed
    }
    
    // Split username:password
    char* colon = strchr(decoded, ':');
    if (!colon) {
        return 0; // Invalid format
    }
    
    *colon = '\0';
    char* username = decoded;
    char* password = colon + 1;
    
    // Check credentials
    if (strcmp(username, required_user) == 0 && strcmp(password, required_pass) == 0) {
        return 1; // Auth successful
    }
    
    return 0; // Auth failed
}

void send_auth_required(int client_fd) {
    const char* response = 
        "HTTP/1.1 401 Unauthorized\r\n"
        "WWW-Authenticate: Basic realm=\"Restricted Area\"\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 87\r\n"
        "\r\n"
        "<html><body><h1>401 Unauthorized</h1><p>Authentication required.</p></body></html>";
    
    write(client_fd, response, strlen(response));
}
```

## Step 5: Protected Route Handler

Add to `src/handlers.c`:

```c
// Protected admin page: GET /admin
int handle_admin_page(http_request_t* request, http_response_t* response) {
    // Check authentication
    if (!check_basic_auth(request, "admin", "secret123")) {
        response->status_code = 401;
        response->body = strdup("<html><body><h1>401 Unauthorized</h1><p>Authentication required.</p></body></html>");
        response->body_length = strlen(response->body);
        strcpy(response->headers[0][0], "Content-Type");
        strcpy(response->headers[0][1], "text/html");
        strcpy(response->headers[1][0], "WWW-Authenticate");
        strcpy(response->headers[1][1], "Basic realm=\"Admin Area\"");
        response->header_count = 2;
        return 0;
    }
    
    const char* html_response = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>Admin Panel</title>\n"
        "    <style>\n"
        "        body { font-family: Arial, sans-serif; margin: 40px; }\n"
        "        .admin { background: #ffe8e8; padding: 20px; border-radius: 5px; border: 2px solid #ff6b6b; }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>Admin Panel</h1>\n"
        "    <div class=\"admin\">\n"
        "        <h2>🔒 Restricted Area</h2>\n"
        "        <p>Welcome to the admin panel! This page requires authentication.</p>\n"
        "        <p><strong>Server Statistics:</strong></p>\n"
        "        <ul>\n"
        "            <li>Uptime: Running</li>\n"
        "            <li>Memory: OK</li>\n"
        "            <li>Connections: Active</li>\n"
        "        </ul>\n"
        "    </div>\n"
        "    <p><a href=\"/\">← Back to Home</a></p>\n"
        "</body>\n"
        "</html>";
    
    set_response(response, 200, "text/html", html_response);
    return 0;
}

// Declare the auth function
int check_basic_auth(http_request_t* request, const char* required_user, const char* required_pass);
```

## Step 6: Enhanced Main Server

Update `src/server.c` for persistent connections:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "../include/http.h"
#include "../include/router.h"

#define PORT 8080
#define CONNECTION_TIMEOUT 30
#define MAX_KEEP_ALIVE_REQUESTS 100

// Declare all handler functions
int handle_api_time(http_request_t* request, http_response_t* response);
int handle_api_echo(http_request_t* request, http_response_t* response);
int handle_contact_form(http_request_t* request, http_response_t* response);
int handle_status_page(http_request_t* request, http_response_t* response);
int handle_contact_page(http_request_t* request, http_response_t* response);
int handle_admin_page(http_request_t* request, http_response_t* response);

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
    add_route(&global_router, "GET", "/admin", handle_admin_page);
}

void handle_connection(int client_fd) {
    connection_t conn;
    init_connection(&conn, client_fd);
    
    int request_count = 0;
    
    while (conn.keep_alive && request_count < MAX_KEEP_ALIVE_REQUESTS) {
        http_request_t request;
        http_response_t response;
        
        init_http_response(&response);
        
        // Set socket timeout
        struct timeval timeout;
        timeout.tv_sec = CONNECTION_TIMEOUT;
        timeout.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        
        int read_result = read_http_request(&conn, &request);
        if (read_result <= 0) {
            if (read_result == 0) {
                // Incomplete request, timeout
                printf("Connection timeout or incomplete request\n");
            }
            free_http_response(&response);
            break;
        }
        
        request_count++;
        
        // Set keep-alive based on request and connection state
        response.keep_alive = conn.keep_alive && (request_count < MAX_KEEP_ALIVE_REQUESTS);
        
        // Handle the request (same logic as before)
        char path[MAX_URI_SIZE];
        strncpy(path, request.uri, sizeof(path) - 1);
        char* query_start = strchr(path, '?');
        if (query_start) {
            *query_start = '\0';
        }
        
        http_request_t route_request = request;
        strncpy(route_request.uri, path, sizeof(route_request.uri) - 1);
        
        if (route_request(global_router, &route_request, &response) == 0) {
            printf("Handled by dynamic route: %s\n", path);
        } else if (strcmp(request.method, "GET") == 0) {
            if (serve_static_file(request.uri, &response) == 0) {
                printf("Served static file: %s\n", request.uri);
            } else {
                response.status_code = 404;
                response.body = strdup("<!DOCTYPE html><html><body><h1>404 - Page Not Found</h1><p><a href=\"/\">Go Home</a></p></body></html>");
                response.body_length = strlen(response.body);
                strcpy(response.headers[0][0], "Content-Type");
                strcpy(response.headers[0][1], "text/html");
                response.header_count = 1;
            }
        } else {
            response.status_code = 405;
            response.body = strdup("Method not allowed");
            response.body_length = strlen(response.body);
            strcpy(response.headers[0][0], "Content-Type");
            strcpy(response.headers[0][1], "text/plain");
            response.header_count = 1;
        }
        
        // Log the request
        log_request(&request, &response);
        
        // Send response
        send_http_response(client_fd, &response);
        
        // Update connection state
        conn.keep_alive = response.keep_alive;
        
        free_http_request(&request);
        free_http_response(&response);
        
        if (!conn.keep_alive) {
            printf("Closing connection (keep-alive disabled)\n");
            break;
        }
    }
    
    if (request_count >= MAX_KEEP_ALIVE_REQUESTS) {
        printf("Closing connection (max requests reached)\n");
    }
}

int main() {
    setup_routes();
    
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    printf("Starting advanced HTTP server on port %d...\n", PORT);
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind and listen
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }
    
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
    
    printf("Advanced HTTP server listening on port %d\n", PORT);
    printf("Features: Keep-Alive, Security Headers, Basic Auth, Logging\n");
    
    while (1) {
        printf("Waiting for connection...\n");
        
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("Client connected from %s\n", inet_ntoa(client_addr.sin_addr));
        
        handle_connection(client_fd);
        
        close(client_fd);
        printf("Client disconnected\n");
    }
    
    close(server_fd);
    return 0;
}
```

## Step 7: Update Build System

Update `Makefile`:

```makefile
CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRCDIR=src
INCDIR=include
SOURCES=$(SRCDIR)/server.c $(SRCDIR)/http_parser.c $(SRCDIR)/file_server.c $(SRCDIR)/router.c $(SRCDIR)/handlers.c $(SRCDIR)/connection.c $(SRCDIR)/auth.c
TARGET=server

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
```

## Step 8: Test Advanced Features

### Compile and Run
```bash
make clean
make
./server
```

### Test Keep-Alive
```bash
# Test multiple requests on same connection
curl -v -H "Connection: keep-alive" http://localhost:8080/
curl -v -H "Connection: keep-alive" http://localhost:8080/status
```

### Test Authentication
```bash
# Try accessing admin without auth (should get 401)
curl http://localhost:8080/admin

# Access with authentication
curl -u admin:secret123 http://localhost:8080/admin
```

### Test in Browser
Visit `http://localhost:8080/admin` - you should see a login prompt!

## What You've Accomplished

Your HTTP server now includes:

✅ **Persistent Connections** - Reuses TCP connections for better performance  
✅ **Security Headers** - Protects against common web vulnerabilities  
✅ **Basic Authentication** - Password-protected areas  
✅ **Request Logging** - Tracks all requests with timestamps  
✅ **Connection Timeouts** - Prevents resource exhaustion  
✅ **Error Handling** - Graceful handling of various error conditions  
✅ **Production Features** - Server headers, proper HTTP status codes  

## Performance Improvements

Compared to our initial server, this version:
- Handles multiple requests per connection (reduces overhead)
- Includes security measures against common attacks
- Provides better error handling and logging
- Supports authentication for protected resources

## Next Steps

Your HTTP server is now quite sophisticated! To make it production-ready, you might consider:
- Multi-threading or async I/O for true concurrency
- SSL/TLS support for HTTPS
- Configuration file support
- Database integration
- More sophisticated routing (regex patterns)
- Middleware system
- Rate limiting
- Compression (gzip)

Congratulations on building a complete HTTP server from scratch!

---

**Previous:** [← Dynamic Content](05-dynamic-content.md) | **Next:** [Testing Guide →](07-testing-guide.md)
