# Phase 2: HTTP Request Parsing

## What We're Building

In this phase, we'll transform our basic socket server into an HTTP server by:
- Understanding HTTP request format
- Parsing HTTP requests into structured data
- Sending proper HTTP responses
- Handling different HTTP methods (GET, POST)

## Understanding HTTP Requests

When a browser visits `http://localhost:8080/hello`, it sends something like this:

```
GET /hello HTTP/1.1
Host: localhost:8080
User-Agent: Mozilla/5.0 (...)
Accept: text/html,application/xhtml+xml
Connection: keep-alive

```

Let's break this down:
- **Request Line**: `GET /hello HTTP/1.1`
  - Method: GET
  - URI: /hello
  - Version: HTTP/1.1
- **Headers**: Key-value pairs with metadata
- **Empty Line**: Separates headers from body
- **Body**: (empty for GET requests)

## Step 1: Define Data Structures

Create `include/http.h`:

```c
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
```

## Step 2: Implement HTTP Parsing

Create `src/http_parser.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/http.h"

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
    const char* header_end = strstr(raw_request, "\r\n\r\n");
    if (!header_end) {
        header_end = strstr(raw_request, "\n\n");
        if (!header_end) {
            return -1; // Malformed request
        }
    }
    
    // Parse request line
    char* line_end = strchr(raw_request, '\n');
    if (!line_end) return -1;
    
    // Copy request line for parsing
    size_t line_length = line_end - raw_request;
    char request_line[line_length + 1];
    strncpy(request_line, raw_request, line_length);
    request_line[line_length] = '\0';
    
    // Remove \r if present
    if (request_line[line_length - 1] == '\r') {
        request_line[line_length - 1] = '\0';
    }
    
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
        
        line_length = line_end - current;
        if (line_length <= 1) break; // Empty line
        
        char header_line[line_length + 1];
        strncpy(header_line, current, line_length);
        header_line[line_length] = '\0';
        
        // Remove \r if present
        if (header_line[line_length - 1] == '\r') {
            header_line[line_length - 1] = '\0';
        }
        
        // Parse header key: value
        char* colon = strchr(header_line, ':');
        if (colon) {
            *colon = '\0';
            char* key = header_line;
            char* value = colon + 1;
            
            // Skip leading whitespace in value
            while (*value == ' ' || *value == '\t') value++;
            
            strncpy(request->headers[request->header_count][0], key, MAX_HEADER_SIZE - 1);
            strncpy(request->headers[request->header_count][1], value, MAX_HEADER_SIZE - 1);
            request->header_count++;
        }
        
        current = line_end + 1;
    }
    
    return 0; // Success
}

int send_http_response(int client_fd, http_response_t* response) {
    // Status line
    char status_line[256];
    const char* status_text = "OK";
    if (response->status_code == 404) status_text = "Not Found";
    else if (response->status_code == 500) status_text = "Internal Server Error";
    
    snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", 
             response->status_code, status_text);
    write(client_fd, status_line, strlen(status_line));
    
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

## Step 3: Update the Main Server

Update `src/server.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/http.h"

#define PORT 8080
#define BUFFER_SIZE 4096

void handle_request(int client_fd, const char* raw_request) {
    http_request_t request;
    http_response_t response;
    
    init_http_response(&response);
    
    if (parse_http_request(raw_request, &request) == 0) {
        printf("Method: %s\n", request.method);
        printf("URI: %s\n", request.uri);
        printf("Version: %s\n", request.version);
        printf("Headers: %d\n", request.header_count);
        
        // Simple routing
        if (strcmp(request.method, "GET") == 0) {
            if (strcmp(request.uri, "/") == 0) {
                response.status_code = 200;
                response.body = strdup("<html><body><h1>Welcome to our HTTP Server!</h1></body></html>");
                response.body_length = strlen(response.body);
                strcpy(response.headers[0][0], "Content-Type");
                strcpy(response.headers[0][1], "text/html");
                response.header_count = 1;
            } else if (strcmp(request.uri, "/hello") == 0) {
                response.status_code = 200;
                response.body = strdup("Hello, World!");
                response.body_length = strlen(response.body);
                strcpy(response.headers[0][0], "Content-Type");
                strcpy(response.headers[0][1], "text/plain");
                response.header_count = 1;
            } else {
                response.status_code = 404;
                response.body = strdup("Page not found");
                response.body_length = strlen(response.body);
                strcpy(response.headers[0][0], "Content-Type");
                strcpy(response.headers[0][1], "text/plain");
                response.header_count = 1;
            }
        } else {
            response.status_code = 405; // Method not allowed
            response.body = strdup("Method not allowed");
            response.body_length = strlen(response.body);
        }
    } else {
        response.status_code = 400; // Bad request
        response.body = strdup("Bad request");
        response.body_length = strlen(response.body);
    }
    
    send_http_response(client_fd, &response);
    
    free_http_request(&request);
    free_http_response(&response);
}

int main() {
    // ... (socket setup code same as before)
    
    while (1) {
        printf("Waiting for connection...\n");
        
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("Client connected from %s\n", inet_ntoa(client_addr.sin_addr));
        
        // Read HTTP request
        char buffer[BUFFER_SIZE];
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
```

## Step 4: Update the Makefile

Update `Makefile`:

```makefile
CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRCDIR=src
INCDIR=include
SOURCES=$(SRCDIR)/server.c $(SRCDIR)/http_parser.c
TARGET=server

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
```

## Step 5: Test Your HTTP Server

### Compile and Run
```bash
make clean
make
./server
```

### Test with curl
```bash
# Test home page
curl http://localhost:8080/

# Test hello endpoint
curl http://localhost:8080/hello

# Test 404 error
curl http://localhost:8080/nonexistent

# See full HTTP response
curl -v http://localhost:8080/
```

### Test with Browser
Open your browser and visit:
- `http://localhost:8080/`
- `http://localhost:8080/hello`

## What You Should See

**For `curl http://localhost:8080/`:**
```html
<html><body><h1>Welcome to our HTTP Server!</h1></body></html>
```

**Server output:**
```
Method: GET
URI: /
Version: HTTP/1.1
Headers: 3
```

## Common Issues

### "Segmentation fault"
- Check that you're not accessing NULL pointers
- Make sure to initialize structures properly
- Use `gdb` to debug: `gdb ./server`

### "Bad request" responses
- Check that your HTTP parsing is handling line endings correctly
- Print the raw request to see what you're receiving

### Browser shows "connection refused"
- Make sure the server is running
- Check that you're using the correct port

## Understanding the Code Flow

1. **Client connects** → Server accepts connection
2. **Client sends HTTP request** → Server reads raw bytes
3. **Parse request** → Extract method, URI, headers
4. **Route request** → Decide what response to send
5. **Build response** → Create HTTP response with headers
6. **Send response** → Write formatted HTTP response
7. **Close connection** → Clean up resources

## Next Steps

Congratulations! You now have a working HTTP server that can:
- Parse HTTP requests
- Route to different endpoints
- Send proper HTTP responses
- Handle errors gracefully

In the next phase, we'll add the ability to serve static files from disk.

---

**Previous:** [← Basic Socket Server](02-basic-socket-server.md) | **Next:** [Phase 3: Static File Serving →](04-static-files.md)
