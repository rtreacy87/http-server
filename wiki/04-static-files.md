# Phase 3: Static File Serving

## What We're Building

In this phase, we'll add the ability to serve static files like HTML, CSS, JavaScript, and images from a directory on your server. This is how most web servers work - they serve files from disk when requested.

## Understanding Static File Serving

When a browser requests `/style.css`, our server should:
1. Look for a file named `style.css` in our document root
2. Read the file from disk
3. Send it back with the correct Content-Type header
4. Handle cases where the file doesn't exist

## Step 1: Create Test Files

First, let's create some test files to serve:

```bash
# Create static directory structure
mkdir -p static/css static/js static/images

# Create a simple HTML file
cat > static/index.html << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>My HTTP Server</title>
    <link rel="stylesheet" href="/css/style.css">
</head>
<body>
    <h1>Welcome to My HTTP Server!</h1>
    <p>This page is served from a static file.</p>
    <img src="/images/logo.png" alt="Logo" style="max-width: 200px;">
    <script src="/js/script.js"></script>
</body>
</html>
EOF

# Create a CSS file
cat > static/css/style.css << 'EOF'
body {
    font-family: Arial, sans-serif;
    max-width: 800px;
    margin: 0 auto;
    padding: 20px;
    background-color: #f5f5f5;
}

h1 {
    color: #333;
    border-bottom: 2px solid #007acc;
    padding-bottom: 10px;
}

p {
    line-height: 1.6;
    color: #666;
}
EOF

# Create a JavaScript file
cat > static/js/script.js << 'EOF'
console.log('Hello from static JavaScript!');
document.addEventListener('DOMContentLoaded', function() {
    console.log('Page loaded successfully');
});
EOF
```

## Step 2: Add File Serving Functions

Create `src/file_server.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../include/http.h"

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
    if (!is_safe_path(uri)) {
        return NULL;
    }
    
    // If URI is just "/", serve index.html
    const char* file_path = uri;
    if (strcmp(uri, "/") == 0) {
        file_path = "/index.html";
    }
    
    // Build full path
    size_t path_len = strlen(DOCUMENT_ROOT) + strlen(file_path) + 1;
    char* full_path = malloc(path_len);
    if (!full_path) {
        return NULL;
    }
    
    snprintf(full_path, path_len, "%s%s", DOCUMENT_ROOT, file_path);
    return full_path;
}

int serve_static_file(const char* uri, http_response_t* response) {
    char* file_path = build_file_path(uri);
    if (!file_path) {
        return -1; // Invalid path
    }
    
    // Check if file exists and get its size
    struct stat file_stat;
    if (stat(file_path, &file_stat) != 0) {
        free(file_path);
        return -1; // File not found
    }
    
    // Check if it's a regular file
    if (!S_ISREG(file_stat.st_mode)) {
        free(file_path);
        return -1; // Not a regular file
    }
    
    // Open file
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        free(file_path);
        return -1; // Cannot open file
    }
    
    // Allocate buffer for file content
    response->body_length = file_stat.st_size;
    response->body = malloc(response->body_length);
    if (!response->body) {
        close(fd);
        free(file_path);
        return -1; // Memory allocation failed
    }
    
    // Read file content
    ssize_t bytes_read = read(fd, response->body, response->body_length);
    close(fd);
    
    if (bytes_read != (ssize_t)response->body_length) {
        free(response->body);
        response->body = NULL;
        free(file_path);
        return -1; // Read error
    }
    
    // Set response headers
    response->status_code = 200;
    
    // Content-Type header
    const char* mime_type = get_mime_type(file_path);
    strcpy(response->headers[response->header_count][0], "Content-Type");
    strcpy(response->headers[response->header_count][1], mime_type);
    response->header_count++;
    
    // Cache-Control header for static files
    strcpy(response->headers[response->header_count][0], "Cache-Control");
    strcpy(response->headers[response->header_count][1], "public, max-age=3600");
    response->header_count++;
    
    free(file_path);
    return 0; // Success
}
```

## Step 3: Update Header File

Add to `include/http.h`:

```c
// Add this function declaration
int serve_static_file(const char* uri, http_response_t* response);
```

## Step 4: Update Main Server

Update the `handle_request` function in `src/server.c`:

```c
void handle_request(int client_fd, const char* raw_request) {
    http_request_t request;
    http_response_t response;
    
    init_http_response(&response);
    
    if (parse_http_request(raw_request, &request) == 0) {
        printf("Method: %s\n", request.method);
        printf("URI: %s\n", request.uri);
        printf("Version: %s\n", request.version);
        
        if (strcmp(request.method, "GET") == 0) {
            // Try to serve static file first
            if (serve_static_file(request.uri, &response) == 0) {
                printf("Served static file: %s\n", request.uri);
            } else {
                // Fall back to dynamic routes
                if (strcmp(request.uri, "/api/hello") == 0) {
                    response.status_code = 200;
                    response.body = strdup("{\"message\": \"Hello from API!\"}");
                    response.body_length = strlen(response.body);
                    strcpy(response.headers[0][0], "Content-Type");
                    strcpy(response.headers[0][1], "application/json");
                    response.header_count = 1;
                } else if (strcmp(request.uri, "/api/time") == 0) {
                    // Dynamic content example
                    time_t now = time(NULL);
                    char* time_str = ctime(&now);
                    time_str[strlen(time_str) - 1] = '\0'; // Remove newline
                    
                    char json_response[256];
                    snprintf(json_response, sizeof(json_response), 
                            "{\"current_time\": \"%s\"}", time_str);
                    
                    response.status_code = 200;
                    response.body = strdup(json_response);
                    response.body_length = strlen(response.body);
                    strcpy(response.headers[0][0], "Content-Type");
                    strcpy(response.headers[0][1], "application/json");
                    response.header_count = 1;
                } else {
                    // 404 Not Found
                    response.status_code = 404;
                    response.body = strdup("<!DOCTYPE html><html><body><h1>404 - Page Not Found</h1></body></html>");
                    response.body_length = strlen(response.body);
                    strcpy(response.headers[0][0], "Content-Type");
                    strcpy(response.headers[0][1], "text/html");
                    response.header_count = 1;
                }
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
```

Don't forget to add the time include at the top of `server.c`:
```c
#include <time.h>
```

## Step 5: Update Makefile

Update `Makefile`:

```makefile
CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRCDIR=src
INCDIR=include
SOURCES=$(SRCDIR)/server.c $(SRCDIR)/http_parser.c $(SRCDIR)/file_server.c
TARGET=server

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
```

## Step 6: Test Your File Server

### Compile and Run
```bash
make clean
make
./server
```

### Test Static Files
```bash
# Test home page (should serve static/index.html)
curl http://localhost:8080/

# Test CSS file
curl http://localhost:8080/css/style.css

# Test JavaScript file
curl http://localhost:8080/js/script.js

# Test API endpoints
curl http://localhost:8080/api/hello
curl http://localhost:8080/api/time

# Test 404
curl http://localhost:8080/nonexistent.html
```

### Test with Browser
Open your browser and visit `http://localhost:8080/`. You should see a styled HTML page!

## Security Features

Our file server includes several security measures:

### Directory Traversal Prevention
```c
int is_safe_path(const char* path) {
    if (strstr(path, "..") != NULL) return 0;  // Prevents ../../../etc/passwd
    if (strstr(path, "//") != NULL) return 0;  // Prevents //etc/passwd
    return 1;
}
```

### File Type Validation
- Only serves regular files (not directories or special files)
- Uses `stat()` to check file properties before serving

### Document Root Restriction
- All files are served relative to `DOCUMENT_ROOT`
- Cannot access files outside the static directory

## Understanding MIME Types

MIME types tell browsers how to handle different file types:
- `text/html` → Browser renders as HTML
- `text/css` → Browser applies as stylesheet
- `application/javascript` → Browser executes as JavaScript
- `image/png` → Browser displays as image

## Common Issues

### "File not found" for existing files
- Check file permissions: `ls -la static/`
- Ensure files are in the correct directory
- Check for typos in filenames

### CSS/JS not loading in browser
- Check browser developer tools (F12) for 404 errors
- Verify MIME types are set correctly
- Check file paths in HTML

### "Permission denied" errors
- Make sure files are readable: `chmod 644 static/*`
- Check directory permissions: `chmod 755 static/`

## Performance Considerations

Our current implementation:
- Reads entire files into memory (fine for small files)
- No caching (files are read from disk every time)
- No compression

For production servers, you'd want:
- Streaming for large files
- File caching in memory
- Gzip compression
- HTTP caching headers

## Next Steps

Great! You now have a web server that can:
- Serve static HTML, CSS, and JavaScript files
- Handle different MIME types correctly
- Provide basic security against directory traversal
- Mix static files with dynamic API endpoints

In the next phase, we'll add more sophisticated dynamic content generation and routing.

---

**Previous:** [← HTTP Request Parsing](03-http-parsing.md) | **Next:** [Phase 4: Dynamic Content →](05-dynamic-content.md)
