# Troubleshooting Guide

## Overview

This guide helps you diagnose and fix common issues when building and running your HTTP server. Issues are organized by category with step-by-step solutions.

## Compilation Issues

### Error: "No such file or directory"

**Symptoms**:
```
fatal error: http.h: No such file or directory
```

**Solutions**:
1. Check include path in Makefile:
   ```makefile
   CFLAGS=-Wall -Wextra -std=c99 -I include
   ```

2. Verify file structure:
   ```
   project/
   ├── include/
   │   ├── http.h
   │   └── router.h
   ├── src/
   │   ├── server.c
   │   └── ...
   └── Makefile
   ```

3. Check include statements:
   ```c
   #include "../include/http.h"  // From src/ directory
   #include "http.h"             // With -I include flag
   ```

### Error: "Undefined reference to function"

**Symptoms**:
```
undefined reference to `parse_http_request'
```

**Solutions**:
1. Check if source file is included in Makefile:
   ```makefile
   SOURCES=$(SRCDIR)/server.c $(SRCDIR)/http_parser.c $(SRCDIR)/router.c
   ```

2. Verify function declaration in header:
   ```c
   // In http.h
   int parse_http_request(const char* raw_request, http_request_t* request);
   ```

3. Check function definition exists:
   ```c
   // In http_parser.c
   int parse_http_request(const char* raw_request, http_request_t* request) {
       // Implementation
   }
   ```

### Warning: "Implicit declaration of function"

**Symptoms**:
```
warning: implicit declaration of function 'strdup'
```

**Solutions**:
1. Add feature test macros:
   ```c
   #define _GNU_SOURCE  // For strdup, strcasecmp
   #include <string.h>
   ```

2. Or use alternative functions:
   ```c
   // Instead of strdup
   char* my_strdup(const char* str) {
       size_t len = strlen(str) + 1;
       char* copy = malloc(len);
       if (copy) strcpy(copy, str);
       return copy;
   }
   ```

## Runtime Issues

### Server Won't Start

#### "Address already in use"

**Symptoms**:
```
bind: Address already in use
```

**Solutions**:
1. Check if port is in use:
   ```bash
   netstat -tulpn | grep :8080
   lsof -i :8080
   ```

2. Kill existing process:
   ```bash
   sudo kill -9 <PID>
   ```

3. Use SO_REUSEADDR option:
   ```c
   int opt = 1;
   setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
   ```

4. Change port number:
   ```c
   #define PORT 8081  // Use different port
   ```

#### "Permission denied"

**Symptoms**:
```
bind: Permission denied
```

**Solutions**:
1. Use port > 1024:
   ```c
   #define PORT 8080  // Instead of 80
   ```

2. Run with elevated privileges (not recommended for development):
   ```bash
   sudo ./server
   ```

3. Check firewall settings:
   ```bash
   sudo ufw status
   sudo iptables -L
   ```

### Server Crashes

#### Segmentation Fault

**Symptoms**:
```
Segmentation fault (core dumped)
```

**Debugging Steps**:
1. Compile with debug symbols:
   ```bash
   gcc -g -Wall -Wextra -std=c99 -I include -o server src/*.c
   ```

2. Run with GDB:
   ```bash
   gdb ./server
   (gdb) run
   (gdb) bt  # When it crashes
   ```

3. Use Valgrind:
   ```bash
   valgrind --leak-check=full ./server
   ```

**Common Causes**:
- NULL pointer dereference
- Buffer overflow
- Use after free
- Stack overflow

#### Memory Leaks

**Detection**:
```bash
valgrind --leak-check=full --show-leak-kinds=all ./server
```

**Common Fixes**:
1. Free allocated memory:
   ```c
   char* body = strdup("Hello");
   // ... use body ...
   free(body);  // Don't forget this!
   ```

2. Free request/response structures:
   ```c
   free_http_request(&request);
   free_http_response(&response);
   ```

3. Check for double-free errors:
   ```c
   if (ptr) {
       free(ptr);
       ptr = NULL;  // Prevent double-free
   }
   ```

## Connection Issues

### Client Can't Connect

**Symptoms**:
- Browser shows "Connection refused"
- `curl` returns "Connection refused"

**Solutions**:
1. Verify server is running:
   ```bash
   ps aux | grep server
   ```

2. Check if server is listening:
   ```bash
   netstat -tulpn | grep :8080
   ```

3. Test locally first:
   ```bash
   curl http://localhost:8080
   telnet localhost 8080
   ```

4. Check firewall:
   ```bash
   sudo ufw allow 8080
   ```

### Connection Drops

**Symptoms**:
- Connections close unexpectedly
- "Connection reset by peer"

**Solutions**:
1. Check for early connection closure:
   ```c
   // Don't close connection too early
   send_http_response(client_fd, &response);
   // Add small delay before closing
   usleep(100000);  // 100ms
   close(client_fd);
   ```

2. Handle SIGPIPE signal:
   ```c
   #include <signal.h>
   
   int main() {
       signal(SIGPIPE, SIG_IGN);  // Ignore broken pipe
       // ... rest of code
   }
   ```

3. Check for buffer overflows:
   ```c
   // Ensure buffers are large enough
   char buffer[BUFFER_SIZE];
   int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
   buffer[bytes_read] = '\0';
   ```

## HTTP Protocol Issues

### Malformed Responses

**Symptoms**:
- Browser shows garbled content
- Missing headers or body

**Solutions**:
1. Check HTTP response format:
   ```c
   // Correct format:
   "HTTP/1.1 200 OK\r\n"
   "Content-Type: text/html\r\n"
   "Content-Length: 13\r\n"
   "\r\n"  // Empty line required
   "Hello, World!"
   ```

2. Verify Content-Length:
   ```c
   response->body_length = strlen(response->body);
   snprintf(content_length, sizeof(content_length), 
            "Content-Length: %zu\r\n", response->body_length);
   ```

3. Use proper line endings:
   ```c
   write(client_fd, "HTTP/1.1 200 OK\r\n", 17);  // \r\n not just \n
   ```

### Parsing Errors

**Symptoms**:
- "Bad request" responses
- Server can't parse valid requests

**Debugging**:
1. Print raw request:
   ```c
   printf("Raw request:\n%s\n", buffer);
   printf("Request length: %d\n", bytes_read);
   ```

2. Check for incomplete requests:
   ```c
   // Look for complete request
   if (!strstr(buffer, "\r\n\r\n") && !strstr(buffer, "\n\n")) {
       // Incomplete request, need more data
       continue;
   }
   ```

3. Handle different line endings:
   ```c
   // Accept both \r\n and \n
   char* line_end = strstr(buffer, "\r\n");
   if (!line_end) {
       line_end = strchr(buffer, '\n');
   }
   ```

## File Serving Issues

### Files Not Found

**Symptoms**:
- 404 errors for existing files
- "No such file or directory"

**Solutions**:
1. Check file permissions:
   ```bash
   ls -la static/
   chmod 644 static/*.html
   chmod 755 static/
   ```

2. Verify file paths:
   ```c
   printf("Looking for file: %s\n", full_path);
   if (access(full_path, R_OK) != 0) {
       perror("File access");
   }
   ```

3. Check document root:
   ```c
   #define DOCUMENT_ROOT "./static"  // Relative to server executable
   ```

### MIME Type Issues

**Symptoms**:
- CSS files not applying styles
- JavaScript files not executing
- Images not displaying

**Solutions**:
1. Check MIME type mapping:
   ```c
   const char* get_mime_type(const char* filename) {
       const char* ext = strrchr(filename, '.');
       if (!ext) return "application/octet-stream";
       
       if (strcasecmp(ext, ".html") == 0) return "text/html";
       if (strcasecmp(ext, ".css") == 0) return "text/css";
       if (strcasecmp(ext, ".js") == 0) return "application/javascript";
       // ... more types
   }
   ```

2. Verify Content-Type header:
   ```bash
   curl -I http://localhost:8080/style.css
   # Should show: Content-Type: text/css
   ```

## Performance Issues

### Slow Response Times

**Symptoms**:
- Long delays before responses
- Timeouts

**Solutions**:
1. Profile with time:
   ```bash
   time curl http://localhost:8080/
   ```

2. Add timing logs:
   ```c
   #include <sys/time.h>
   
   struct timeval start, end;
   gettimeofday(&start, NULL);
   // ... process request ...
   gettimeofday(&end, NULL);
   
   long duration = (end.tv_sec - start.tv_sec) * 1000000 + 
                   (end.tv_usec - start.tv_usec);
   printf("Request took %ld microseconds\n", duration);
   ```

3. Optimize file I/O:
   ```c
   // Read file in chunks instead of all at once
   #define CHUNK_SIZE 8192
   char chunk[CHUNK_SIZE];
   while ((bytes_read = read(fd, chunk, CHUNK_SIZE)) > 0) {
       write(client_fd, chunk, bytes_read);
   }
   ```

### Memory Usage

**Symptoms**:
- Server uses too much memory
- Out of memory errors

**Solutions**:
1. Monitor memory usage:
   ```bash
   top -p $(pgrep server)
   valgrind --tool=massif ./server
   ```

2. Limit request sizes:
   ```c
   #define MAX_REQUEST_SIZE (1024 * 1024)  // 1MB limit
   
   if (bytes_read > MAX_REQUEST_SIZE) {
       // Reject large requests
       send_error_response(client_fd, 413);  // Payload too large
       return;
   }
   ```

3. Reuse buffers:
   ```c
   static char reusable_buffer[BUFFER_SIZE];
   // Reuse instead of malloc/free
   ```

## Debugging Tools and Techniques

### Enable Debug Output

```c
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

// Compile with: gcc -DDEBUG ...
```

### Network Debugging

```bash
# Monitor network traffic
sudo tcpdump -i lo port 8080

# Test with different tools
curl -v http://localhost:8080/
wget -O - http://localhost:8080/
nc localhost 8080  # Manual HTTP
```

### System Call Tracing

```bash
# Trace system calls
strace -e trace=network ./server
strace -e trace=file ./server
```

## Common Error Patterns

### Pattern 1: Null Pointer Access
```c
// Bad
char* header_value = get_header_value(request, "Host");
int len = strlen(header_value);  // Crash if NULL

// Good
char* header_value = get_header_value(request, "Host");
if (header_value) {
    int len = strlen(header_value);
}
```

### Pattern 2: Buffer Overflow
```c
// Bad
char buffer[256];
strcpy(buffer, user_input);

// Good
char buffer[256];
strncpy(buffer, user_input, sizeof(buffer) - 1);
buffer[sizeof(buffer) - 1] = '\0';
```

### Pattern 3: Resource Leaks
```c
// Bad
FILE* file = fopen("test.txt", "r");
if (some_condition) {
    return;  // File not closed!
}
fclose(file);

// Good
FILE* file = fopen("test.txt", "r");
if (some_condition) {
    fclose(file);
    return;
}
fclose(file);
```

## Getting Help

### Information to Gather
1. **Error messages**: Exact text of errors
2. **System info**: OS, compiler version
3. **Code snippets**: Minimal reproducing example
4. **Steps to reproduce**: What you did before the error
5. **Expected vs actual**: What should happen vs what does

### Useful Commands
```bash
# System information
uname -a
gcc --version
ldd ./server

# Process information
ps aux | grep server
lsof -p <server_pid>

# Network information
netstat -tulpn | grep server
ss -tulpn | grep :8080
```

### Online Resources
- Stack Overflow (tag: c, http, sockets)
- C reference documentation
- HTTP specification (RFC 7230-7237)
- POSIX socket documentation

Remember: Most issues are caused by simple mistakes like missing null terminators, incorrect buffer sizes, or forgetting to free memory. Start with the basics and work your way up to more complex debugging!

---

**Previous:** [← Security Guide](08-security-guide.md) | **Next:** [Project Summary →](10-project-summary.md)
