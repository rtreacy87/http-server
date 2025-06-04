# Security Best Practices for HTTP Servers

## Overview

Security is crucial for any web server. This guide covers common vulnerabilities, attack vectors, and how to protect your HTTP server against them. While our server includes basic security measures, understanding these concepts is essential for building secure web applications.

## Common Web Vulnerabilities

### 1. Directory Traversal (Path Traversal)

**What it is**: Attackers try to access files outside the web root using `../` sequences.

**Example Attack**:
```
GET /../../../etc/passwd HTTP/1.1
```

**Our Protection**:
```c
int is_safe_path(const char* path) {
    if (strstr(path, "..") != NULL) return 0;  // Block ../ sequences
    if (strstr(path, "//") != NULL) return 0;  // Block double slashes
    return 1;
}
```

**Additional Protections**:
- Canonicalize paths before checking
- Use chroot jails in production
- Validate file extensions

### 2. Buffer Overflow

**What it is**: Writing more data to a buffer than it can hold, potentially overwriting memory.

**Example Vulnerable Code**:
```c
char buffer[256];
strcpy(buffer, user_input);  // Dangerous if user_input > 255 chars
```

**Our Protection**:
```c
strncpy(request->uri, token, MAX_URI_SIZE - 1);  // Limit copy size
request->uri[MAX_URI_SIZE - 1] = '\0';           // Ensure null termination
```

**Best Practices**:
- Always use safe string functions (`strncpy`, `snprintf`)
- Check buffer bounds before writing
- Use static analysis tools

### 3. HTTP Header Injection

**What it is**: Injecting malicious headers through user input.

**Example Attack**:
```
GET /search?q=test%0D%0ASet-Cookie:%20admin=true HTTP/1.1
```

**Protection Strategy**:
```c
void sanitize_header_value(char* value) {
    // Remove CR/LF characters
    for (int i = 0; value[i]; i++) {
        if (value[i] == '\r' || value[i] == '\n') {
            value[i] = ' ';
        }
    }
}
```

### 4. Denial of Service (DoS)

**What it is**: Overwhelming the server with requests or large payloads.

**Our Protections**:
```c
#define MAX_REQUEST_SIZE (1024 * 1024)  // 1MB limit
#define CONNECTION_TIMEOUT 30           // 30 second timeout
#define MAX_KEEP_ALIVE_REQUESTS 100     // Limit requests per connection
```

**Additional Protections**:
- Rate limiting per IP
- Connection limits
- Request size validation

## Security Headers

### Essential Security Headers

Our server includes these security headers:

```c
void add_security_headers(http_response_t* response) {
    // Prevent MIME type sniffing
    strcpy(response->headers[response->header_count][0], "X-Content-Type-Options");
    strcpy(response->headers[response->header_count][1], "nosniff");
    response->header_count++;
    
    // Prevent clickjacking
    strcpy(response->headers[response->header_count][0], "X-Frame-Options");
    strcpy(response->headers[response->header_count][1], "DENY");
    response->header_count++;
    
    // XSS protection
    strcpy(response->headers[response->header_count][0], "X-XSS-Protection");
    strcpy(response->headers[response->header_count][1], "1; mode=block");
    response->header_count++;
}
```

### Additional Recommended Headers

For production servers, consider adding:

```c
// Content Security Policy
"Content-Security-Policy: default-src 'self'"

// Strict Transport Security (HTTPS only)
"Strict-Transport-Security: max-age=31536000; includeSubDomains"

// Referrer Policy
"Referrer-Policy: strict-origin-when-cross-origin"

// Permissions Policy
"Permissions-Policy: geolocation=(), microphone=(), camera=()"
```

## Input Validation and Sanitization

### URL Validation

```c
int validate_uri(const char* uri) {
    // Check length
    if (strlen(uri) > MAX_URI_SIZE) {
        return 0;
    }
    
    // Check for null bytes
    if (strchr(uri, '\0') != uri + strlen(uri)) {
        return 0;
    }
    
    // Check for dangerous characters
    const char* dangerous = "<>\"'&";
    for (int i = 0; dangerous[i]; i++) {
        if (strchr(uri, dangerous[i])) {
            return 0;
        }
    }
    
    return 1;
}
```

### Form Data Validation

```c
int validate_form_field(const char* field_name, const char* value) {
    // Email validation
    if (strcmp(field_name, "email") == 0) {
        if (!strchr(value, '@') || strlen(value) > 254) {
            return 0;
        }
    }
    
    // Name validation
    if (strcmp(field_name, "name") == 0) {
        if (strlen(value) > 100 || strlen(value) == 0) {
            return 0;
        }
        // Check for HTML/script tags
        if (strstr(value, "<script") || strstr(value, "<iframe")) {
            return 0;
        }
    }
    
    return 1;
}
```

## Authentication Security

### Secure Password Handling

**Never store passwords in plain text!** Here's a basic example using a simple hash:

```c
#include <openssl/sha.h>

void hash_password(const char* password, const char* salt, char* hash_output) {
    char salted_password[512];
    snprintf(salted_password, sizeof(salted_password), "%s%s", password, salt);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)salted_password, strlen(salted_password), hash);
    
    // Convert to hex string
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_output + (i * 2), "%02x", hash[i]);
    }
}

int verify_password(const char* password, const char* salt, const char* stored_hash) {
    char computed_hash[65]; // 64 hex chars + null terminator
    hash_password(password, salt, computed_hash);
    return strcmp(computed_hash, stored_hash) == 0;
}
```

### Session Management

For production applications, implement proper session management:

```c
typedef struct {
    char session_id[64];
    char username[64];
    time_t created;
    time_t last_access;
    int is_admin;
} session_t;

char* generate_session_id() {
    // Use cryptographically secure random number generator
    // This is a simplified example
    static char session_id[65];
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    
    for (int i = 0; i < 64; i++) {
        session_id[i] = chars[rand() % strlen(chars)];
    }
    session_id[64] = '\0';
    
    return session_id;
}
```

## HTTPS and TLS

### Why HTTPS is Essential

- **Encryption**: Protects data in transit
- **Authentication**: Verifies server identity
- **Integrity**: Prevents tampering

### Adding TLS Support (Advanced)

For production servers, you'll need TLS. Here's a basic OpenSSL integration outline:

```c
#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_CTX* create_ssl_context() {
    const SSL_METHOD* method = TLS_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    
    // Load certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    return ctx;
}
```

## Logging and Monitoring

### Security Event Logging

```c
void log_security_event(const char* event_type, const char* client_ip, const char* details) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    FILE* security_log = fopen("security.log", "a");
    if (security_log) {
        fprintf(security_log, "[%s] SECURITY: %s from %s - %s\n", 
                timestamp, event_type, client_ip, details);
        fclose(security_log);
    }
}

// Usage examples:
log_security_event("AUTH_FAILURE", client_ip, "Invalid credentials for admin");
log_security_event("PATH_TRAVERSAL", client_ip, "Attempted access to ../../../etc/passwd");
log_security_event("LARGE_REQUEST", client_ip, "Request size exceeded limit");
```

### Rate Limiting

```c
#define MAX_REQUESTS_PER_MINUTE 60

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int request_count;
    time_t window_start;
} rate_limit_entry_t;

int check_rate_limit(const char* client_ip) {
    static rate_limit_entry_t entries[100];
    static int entry_count = 0;
    
    time_t now = time(NULL);
    
    // Find existing entry for this IP
    for (int i = 0; i < entry_count; i++) {
        if (strcmp(entries[i].ip, client_ip) == 0) {
            // Reset window if more than a minute has passed
            if (now - entries[i].window_start >= 60) {
                entries[i].request_count = 1;
                entries[i].window_start = now;
                return 1; // Allow
            }
            
            entries[i].request_count++;
            if (entries[i].request_count > MAX_REQUESTS_PER_MINUTE) {
                log_security_event("RATE_LIMIT", client_ip, "Too many requests");
                return 0; // Block
            }
            return 1; // Allow
        }
    }
    
    // New IP, add entry
    if (entry_count < 100) {
        strcpy(entries[entry_count].ip, client_ip);
        entries[entry_count].request_count = 1;
        entries[entry_count].window_start = now;
        entry_count++;
    }
    
    return 1; // Allow
}
```

## File Upload Security

If you add file upload functionality:

### Validate File Types

```c
int is_allowed_file_type(const char* filename) {
    const char* allowed_extensions[] = {".jpg", ".jpeg", ".png", ".gif", ".pdf", ".txt", NULL};
    
    const char* extension = strrchr(filename, '.');
    if (!extension) return 0;
    
    for (int i = 0; allowed_extensions[i]; i++) {
        if (strcasecmp(extension, allowed_extensions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}
```

### Scan File Content

```c
int scan_file_content(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) return 0;
    
    char buffer[1024];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);
    
    // Check for script tags in uploaded files
    if (memmem(buffer, bytes_read, "<script", 7) ||
        memmem(buffer, bytes_read, "<?php", 5) ||
        memmem(buffer, bytes_read, "#!/bin/", 7)) {
        return 0; // Suspicious content
    }
    
    return 1; // Clean
}
```

## Security Checklist

### Development Phase
- [ ] Use safe string functions (strncpy, snprintf)
- [ ] Validate all input data
- [ ] Implement proper error handling
- [ ] Add security headers
- [ ] Use static analysis tools
- [ ] Test with security scanners

### Deployment Phase
- [ ] Use HTTPS in production
- [ ] Set up proper file permissions
- [ ] Configure firewall rules
- [ ] Enable security logging
- [ ] Implement rate limiting
- [ ] Regular security updates

### Ongoing Maintenance
- [ ] Monitor security logs
- [ ] Regular vulnerability scans
- [ ] Keep dependencies updated
- [ ] Review and update security policies
- [ ] Incident response plan

## Tools for Security Testing

### Static Analysis
```bash
# Cppcheck for C code analysis
cppcheck --enable=all src/

# Clang static analyzer
scan-build make
```

### Dynamic Analysis
```bash
# Valgrind for memory errors
valgrind --tool=memcheck ./server

# AddressSanitizer
gcc -fsanitize=address -g -o server src/*.c
```

### Web Security Scanners
```bash
# OWASP ZAP (GUI tool)
# Nikto web scanner
nikto -h http://localhost:8080

# Nmap for port scanning
nmap -sV localhost
```

## Common Security Mistakes to Avoid

1. **Trusting User Input**: Always validate and sanitize
2. **Ignoring Buffer Bounds**: Use safe string functions
3. **Weak Authentication**: Implement proper password policies
4. **Missing Security Headers**: Add all recommended headers
5. **Verbose Error Messages**: Don't leak system information
6. **Default Configurations**: Change default passwords and settings
7. **Insufficient Logging**: Log security events for monitoring
8. **Outdated Dependencies**: Keep libraries updated

## Resources for Further Learning

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [CWE (Common Weakness Enumeration)](https://cwe.mitre.org/)
- [NIST Cybersecurity Framework](https://www.nist.gov/cyberframework)
- [Secure Coding Practices](https://owasp.org/www-project-secure-coding-practices-quick-reference-guide/)

Remember: Security is an ongoing process, not a one-time implementation. Stay informed about new vulnerabilities and best practices!

---

**Previous:** [← Testing Guide](07-testing-guide.md) | **Next:** [Troubleshooting →](09-troubleshooting.md)
