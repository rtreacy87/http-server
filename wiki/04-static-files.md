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

```
The line `#define DOCUMENT_ROOT "./static"` is a preprocessor directive that defines a constant named `DOCUMENT_ROOT` with the value `"./static"`.

Here's what it's doing:

1. **Defines a constant**: Creates a named constant that can be used throughout the code
2. **Sets the document root path**: Specifies that `"./static"` is the base directory for all static files
3. **Relative path**: The `./` prefix means "relative to the current working directory"

In your HTTP server, this constant is used to determine where to look for static files when a client requests them. For example:

- When a browser requests `/index.html`, your server will look for `./static/index.html`
- When a browser requests `/css/style.css`, your server will look for `./static/css/style.css`
- When a browser requests `/images/logo.png`, your server will look for `./static/images/logo.png`

This is used in the `build_file_path` function:

```c
char* build_file_path(const char* uri) {
    // ...
    
    // Build full path
    size_t path_len = strlen(DOCUMENT_ROOT) + strlen(file_path) + 1;
    char* full_path = malloc(path_len);
    if (!full_path) {
        return NULL;
    }
    
    snprintf(full_path, path_len, "%s%s", DOCUMENT_ROOT, file_path);
    return full_path;
}
```

The function combines the document root with the requested URI to create the full file path on your server's filesystem.

Using a defined constant like this is good practice because:
1. It centralizes the configuration - you only need to change it in one place
2. It makes the code more readable - `DOCUMENT_ROOT` is more descriptive than `"./static"`
3. It allows for easy changes - you could change it to an absolute path if needed

Make sure the `static` directory exists in the same directory where you run your server, or modify the `DOCUMENT_ROOT` to point to the correct location.


```c
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

```
The line `{NULL, "application/octet-stream"} // Default` in your MIME type mapping array serves as a fallback or default MIME type when no specific file extension is matched.

Let's break it down:

1. `NULL` as the extension means "no specific extension" or "end of list"
2. `"application/octet-stream"` is the MIME type that will be used as the default

`application/octet-stream` is the standard MIME type for arbitrary binary data. When a browser receives content with this MIME type, it typically:

1. Doesn't try to render or display the content
2. Prompts the user to download the file
3. Treats it as a generic binary file

This is used as the default for several reasons:

1. **Safety**: It's safer to treat unknown file types as generic binary data rather than trying to render them
2. **Compatibility**: All browsers know how to handle this MIME type
3. **Fallback mechanism**: It ensures that even if a file extension isn't recognized, the server can still serve the file
4. **Sentinel value**: The NULL extension also serves as a marker for the end of the array when iterating through it

In your code, this default is used in two places:

1. In the array definition as the last element:
```c
static mime_mapping_t mime_types[] = {
    // ... other MIME types ...
    {NULL, "application/octet-stream"} // Default
};
```

2. In the `get_mime_type` function as a fallback:
```c
const char* get_mime_type(const char* filename) {
    const char* extension = strrchr(filename, '.');
    if (!extension) {
        return "application/octet-stream";  // Default if no extension
    }
    
    // ... check for known extensions ...
    
    return "application/octet-stream";  // Default if extension not recognized
}
```

This ensures that your server can handle any file type, even if it doesn't have a specific MIME type mapping defined.

**What is a MIME type?**

A MIME type (Multipurpose Internet Mail Extensions type) is a standard that indicates the nature and format of a document, file, or assortment of bytes. It's a label that tells software how to interpret and handle the data.

## Key Points About MIME Types:

1. **Purpose**: MIME types tell browsers and other software how to handle different types of files
   
2. **Format**: MIME types have two parts separated by a slash:
   - **Type**: The general category (e.g., text, image, application)
   - **Subtype**: The specific format (e.g., html, jpeg, pdf)
   
   Example: `text/html` or `image/png`

3. **Common Examples**:
   - `text/html` - HTML documents
   - `text/css` - CSS stylesheets
   - `application/javascript` - JavaScript code
   - `image/jpeg` - JPEG images
   - `application/pdf` - PDF documents
   - `application/json` - JSON data
   - `application/octet-stream` - Binary data (default)

4. **Importance in Web Servers**:
   - When a server sends a file to a browser, it includes a `Content-Type` header with the MIME type
   - This header tells the browser how to process and display the content
   - Without the correct MIME type, browsers might:
     - Display binary data as text
     - Download HTML instead of rendering it
     - Fail to apply CSS styles
     - Refuse to execute JavaScript

5. **Default Type**:
   - `application/octet-stream` is used when the type is unknown
   - It signals "this is binary data, handle with caution"
   - Browsers typically offer to download files with this MIME type

In your HTTP server, setting the correct MIME type for each file is crucial for proper functioning of web pages. That's why you have the `get_mime_type()` function that maps file extensions to their appropriate MIME types.



```c
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

```
Let's analyze the loop from the selected code:

```c
for (int i = 0; mime_types[i].extension; i++) {
    if (strcasecmp(extension, mime_types[i].extension) == 0) {
        return mime_types[i].mime_type;
    }
}
```

This loop is searching through the `mime_types` array to find a matching file extension and return its corresponding MIME type. Here's how it works:

1. **Loop Initialization**: `int i = 0` - Starts with the first element of the array

2. **Loop Condition**: `mime_types[i].extension` - Continues as long as the extension field is not NULL
   - This is a clever use of C's truth evaluation - any non-NULL pointer is considered "true"
   - When it reaches the last element `{NULL, "application/octet-stream"}`, the condition becomes false and the loop ends

3. **Loop Increment**: `i++` - Moves to the next element in the array

4. **Inside the Loop**:
   - `strcasecmp(extension, mime_types[i].extension) == 0` - Compares the file's extension with the current array element's extension
   - `strcasecmp` is a case-insensitive string comparison function (e.g., ".JPG" will match ".jpg")
   - If they match (comparison returns 0), it returns the corresponding MIME type

5. **Early Return**: If a match is found, the function immediately returns the MIME type without continuing the loop

If the loop completes without finding a match, the function will continue to the line after the loop, which returns the default MIME type "application/octet-stream".

This is an efficient pattern for searching through a NULL-terminated array of structures, using the NULL value both as a loop terminator and as a marker for the default value.


```c
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

```
The `is_safe_path` function is a critical security measure that helps protect your server from directory traversal attacks. Here's how it works and why it's important:

```c
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
```

## How it helps:

1. **Prevents Directory Traversal Attacks**
   - Directory traversal (also known as path traversal) is a type of attack where an attacker tries to access files outside the intended directory
   - By using sequences like `../` (parent directory), attackers can potentially access sensitive files on your server

2. **Blocks Dangerous Path Patterns**
   - `..` - The function checks if the path contains double dots, which are used to navigate to parent directories
   - `//` - Double slashes can sometimes be used to bypass security filters or cause unexpected behavior in path resolution

3. **Acts as a Gatekeeper**
   - This function is called by `build_file_path` before constructing the full file path
   - If `is_safe_path` returns 0 (false), `build_file_path` returns NULL, preventing access to the requested file

## Example attack scenarios it prevents:

1. **Accessing System Files**:
   - Request: `/../../etc/passwd`
   - Without protection: Server might serve the system password file
   - With `is_safe_path`: Returns 0, request is rejected

2. **Accessing Configuration Files**:
   - Request: `/../config/database.ini`
   - Without protection: Server might expose database credentials
   - With `is_safe_path`: Returns 0, request is rejected

3. **Bypassing Restrictions**:
   - Request: `//etc/shadow`
   - Without protection: Some systems might interpret this in unexpected ways
   - With `is_safe_path`: Returns 0, request is rejected

This simple function is a fundamental security measure that helps ensure your server only serves files from the intended directory, protecting your system from a common class of web vulnerabilities.

## Return Value Conventions

In C programming, there are two common conventions for return values:

1. **UNIX/POSIX Convention**: 
   - 0 means success
   - Non-zero values (often -1) indicate errors
   - Used by most system calls and standard library functions
   - Examples: `fork()`, `open()`, `close()`, `read()`, `write()`

2. **Boolean Convention**:
   - 0 means false (failure)
   - Non-zero (often 1) means true (success)
   - Used for functions that answer yes/no questions
   - Examples: `isalpha()`, `isdigit()`, `strcmp() == 0`

The `is_safe_path` function follows the Boolean convention because:

1. It's answering a yes/no question: "Is this path safe?"
2. The name starts with "is_", suggesting a Boolean result
3. It's more intuitive to read in conditionals:
   ```c
   if (is_safe_path(uri)) {
       // Path is safe, proceed
   } else {
       // Path is unsafe, reject
   }
   ```

This creates a slight inconsistency with functions like `serve_static_file()` which follow the UNIX convention (0 for success, -1 for failure). However, this mixed approach is common in C code:

- Functions that perform operations typically use 0 for success
- Functions that check conditions typically use non-zero for true/success

The key is to be clear in function naming and documentation about which convention is being used. The "is_" prefix helps signal that this function follows the Boolean convention.


The following code is in `src/build_file_path_supplement.c`:

```c
const char* get_default_file_path(const char* uri) {
    // If URI is just "/", serve index.html
    if (strcmp(uri, "/") == 0) {
        return "/index.html";
    }
    return uri;
}

char* construct_full_path(const char* base_dir, const char* file_path) {
    // Calculate required buffer size (+1 for null terminator)
    size_t path_len = strlen(base_dir) + strlen(file_path) + 1;
    
    // Allocate memory for the full path
    char* full_path = malloc(path_len);
    if (!full_path) {
        return NULL;  // Memory allocation failed
    }
    
    // Combine base directory and file path
    snprintf(full_path, path_len, "%s%s", base_dir, file_path);
    return full_path;
```


```c
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
```

```c
// Check if a file exists and is a regular file
int validate_file(const char* file_path, struct stat* file_stat) {
    // Check if file exists and get its size
    if (stat(file_path, file_stat) != 0) {
        return -1; // File not found
    }
    
    // Check if it's a regular file
    if (!S_ISREG(file_stat->st_mode)) {
        return -1; // Not a regular file
    }
    
    return 0; // File is valid
}

// Read file content into memory
void* read_file_content(const char* file_path, size_t file_size, ssize_t* bytes_read) {
    // Open file
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        return NULL; // Cannot open file
    }
    
    // Allocate buffer for file content
    void* content = malloc(file_size);
    if (!content) {
        close(fd);
        return NULL; // Memory allocation failed
    }
    
    // Read file content
    *bytes_read = read(fd, content, file_size);
    close(fd);
    
    if (*bytes_read != (ssize_t)file_size) {
        free(content);
        return NULL; // Read error
    }
    
    return content;
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

// Main function to serve a static file
int serve_static_file(const char* uri, http_response_t* response) {
    char* file_path = build_file_path(uri);
    if (!file_path) {
        return -1; // Invalid path
    }
    
    // Validate the file
    struct stat file_stat;
    if (validate_file(file_path, &file_stat) != 0) {
        free(file_path);
        return -1; // File validation failed
    }
    
    // Read the file content
    ssize_t bytes_read;
    response->body = read_file_content(file_path, file_stat.st_size, &bytes_read);
    if (!response->body) {
        free(file_path);
        return -1; // Failed to read file
    }
    response->body_length = bytes_read;
    
    // Set response status and headers
    response->status_code = 200;
    set_static_file_headers(response, file_path);
    
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
The warning is about a type mismatch between `ssize_t*` and `size_t*`. Let me explain the difference and how to fix it:

## Difference between `size_t` and `ssize_t`:

1. **`size_t`**:
   - Unsigned integer type (always positive)
   - Used for sizes and counts
   - Defined in `<stddef.h>`
   - Common in memory allocation functions like `malloc()`

2. **`ssize_t`**:
   - Signed integer type (can be negative)
   - Used in POSIX functions that need to return -1 on error
   - Defined in `<sys/types.h>`
   - Common in I/O functions like `read()` and `write()`

The warning occurs because you're passing a pointer to a signed type (`ssize_t*`) to a function expecting a pointer to an unsigned type (`size_t*`). This can cause issues if negative values are stored or if the function assumes values are always positive.

## How to fix it:

You have two options:

### Option 1: Change the variable type to match the function parameter

````c path=src/file_server.c mode=EDIT
// Change this:
ssize_t bytes_read;
response->body = read_file_content(file_path, file_stat.st_size, &bytes_read);

// To this:
size_t bytes_read;
response->body = read_file_content(file_path, file_stat.st_size, &bytes_read);
````

### Option 2: Change the function parameter type to match your variable

If you control the `read_file_content` function and need to handle negative values (e.g., for error conditions), modify its declaration:

````c path=src/file_server.c mode=EDIT
// Change this:
char* read_file_content(const char* path, size_t file_size, size_t* bytes_read);

// To this:
char* read_file_content(const char* path, size_t file_size, ssize_t* bytes_read);
````

Option 1 is usually preferred if you don't need negative values, as `size_t` is the standard type for sizes in C. If you're using `bytes_read` only to store the number of bytes successfully read (which should never be negative), then `size_t` is more appropriate.

If you're using the POSIX `read()` function internally (which returns `ssize_t`), you might need to do a safe conversion:

````c path=src/file_server.c mode=EDIT
// Inside read_file_content function:
ssize_t result = read(fd, buffer, count);
if (result >= 0) {
    *bytes_read = (size_t)result;  // Safe conversion (we know it's non-negative)
} else {
    // Handle error
    *bytes_read = 0;
    return NULL;
}
`
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
