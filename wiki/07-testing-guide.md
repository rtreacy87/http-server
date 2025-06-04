# Testing and Debugging Guide

## Overview

Testing is crucial for ensuring your HTTP server works correctly and handles edge cases gracefully. This guide covers various testing approaches, debugging techniques, and common issues you might encounter.

## Types of Testing

### 1. Unit Testing
Test individual components in isolation.

### 2. Integration Testing
Test how components work together.

### 3. Load Testing
Test performance under heavy load.

### 4. Security Testing
Test for vulnerabilities and attack vectors.

## Setting Up a Test Environment

### Create Test Directory Structure
```bash
mkdir tests
mkdir tests/unit
mkdir tests/integration
mkdir tests/load
mkdir tests/data
```

### Create Test Data Files
```bash
# Create test files
echo "Hello World" > tests/data/test.txt
echo '{"test": true}' > tests/data/test.json

# Create large test file
dd if=/dev/zero of=tests/data/large.txt bs=1M count=10
```

## Unit Testing

### Test HTTP Parser

Create `tests/unit/test_parser.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../include/http.h"

void test_parse_get_request() {
    printf("Testing GET request parsing...\n");
    
    const char* raw_request = 
        "GET /test HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: Test\r\n"
        "\r\n";
    
    http_request_t request;
    int result = parse_http_request(raw_request, &request);
    
    assert(result == 0);
    assert(strcmp(request.method, "GET") == 0);
    assert(strcmp(request.uri, "/test") == 0);
    assert(strcmp(request.version, "HTTP/1.1") == 0);
    assert(request.header_count >= 2);
    
    free_http_request(&request);
    printf("✓ GET request parsing test passed\n");
}

void test_parse_post_request() {
    printf("Testing POST request parsing...\n");
    
    const char* raw_request = 
        "POST /api/data HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "{\"test\":true}";
    
    http_request_t request;
    int result = parse_http_request(raw_request, &request);
    
    assert(result == 0);
    assert(strcmp(request.method, "POST") == 0);
    assert(strcmp(request.uri, "/api/data") == 0);
    assert(request.body != NULL);
    
    free_http_request(&request);
    printf("✓ POST request parsing test passed\n");
}

void test_malformed_request() {
    printf("Testing malformed request handling...\n");
    
    const char* raw_request = "INVALID REQUEST";
    
    http_request_t request;
    int result = parse_http_request(raw_request, &request);
    
    assert(result != 0); // Should fail
    
    free_http_request(&request);
    printf("✓ Malformed request test passed\n");
}

int main() {
    printf("Running HTTP Parser Unit Tests\n");
    printf("==============================\n");
    
    test_parse_get_request();
    test_parse_post_request();
    test_malformed_request();
    
    printf("\n✓ All parser tests passed!\n");
    return 0;
}
```

### Test Router

Create `tests/unit/test_router.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../include/router.h"

int test_handler(http_request_t* request, http_response_t* response) {
    (void)request; // Unused
    response->status_code = 200;
    response->body = strdup("Test response");
    response->body_length = strlen(response->body);
    return 0;
}

void test_route_registration() {
    printf("Testing route registration...\n");
    
    router_t router;
    init_router(&router);
    
    int result = add_route(&router, "GET", "/test", test_handler);
    assert(result == 0);
    assert(router.route_count == 1);
    
    printf("✓ Route registration test passed\n");
}

void test_route_matching() {
    printf("Testing route matching...\n");
    
    router_t router;
    init_router(&router);
    add_route(&router, "GET", "/test", test_handler);
    
    http_request_t request;
    http_response_t response;
    init_http_request(&request);
    init_http_response(&response);
    
    strcpy(request.method, "GET");
    strcpy(request.uri, "/test");
    
    int result = route_request(&router, &request, &response);
    assert(result == 0);
    assert(response.status_code == 200);
    
    free_http_request(&request);
    free_http_response(&response);
    printf("✓ Route matching test passed\n");
}

int main() {
    printf("Running Router Unit Tests\n");
    printf("=========================\n");
    
    test_route_registration();
    test_route_matching();
    
    printf("\n✓ All router tests passed!\n");
    return 0;
}
```

### Compile and Run Unit Tests

Create `tests/Makefile`:

```makefile
CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRCDIR=../src
INCDIR=../include

# Source files needed for tests
PARSER_SOURCES=$(SRCDIR)/http_parser.c
ROUTER_SOURCES=$(SRCDIR)/router.c $(SRCDIR)/http_parser.c

test_parser: unit/test_parser.c $(PARSER_SOURCES)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test_parser unit/test_parser.c $(PARSER_SOURCES)

test_router: unit/test_router.c $(ROUTER_SOURCES)
	$(CC) $(CFLAGS) -I$(INCDIR) -o test_router unit/test_router.c $(ROUTER_SOURCES)

run_unit_tests: test_parser test_router
	@echo "Running unit tests..."
	./test_parser
	./test_router

clean:
	rm -f test_parser test_router

.PHONY: run_unit_tests clean
```

Run the tests:
```bash
cd tests
make run_unit_tests
```

## Integration Testing

### Test Complete Server Functionality

Create `tests/integration/test_server.sh`:

```bash
#!/bin/bash

SERVER_PID=""
SERVER_PORT=8080
BASE_URL="http://localhost:$SERVER_PORT"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Start server
start_server() {
    echo -e "${YELLOW}Starting server...${NC}"
    cd ../..
    ./server &
    SERVER_PID=$!
    sleep 2
    
    # Check if server is running
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo -e "${RED}Failed to start server${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}Server started with PID $SERVER_PID${NC}"
}

# Stop server
stop_server() {
    if [ ! -z "$SERVER_PID" ]; then
        echo -e "${YELLOW}Stopping server...${NC}"
        kill $SERVER_PID
        wait $SERVER_PID 2>/dev/null
        echo -e "${GREEN}Server stopped${NC}"
    fi
}

# Test function
test_endpoint() {
    local method=$1
    local endpoint=$2
    local expected_status=$3
    local description=$4
    local data=$5
    
    echo -n "Testing $description... "
    
    if [ "$method" = "POST" ] && [ ! -z "$data" ]; then
        response=$(curl -s -w "%{http_code}" -X POST -d "$data" "$BASE_URL$endpoint")
    else
        response=$(curl -s -w "%{http_code}" "$BASE_URL$endpoint")
    fi
    
    status_code="${response: -3}"
    
    if [ "$status_code" = "$expected_status" ]; then
        echo -e "${GREEN}✓ PASS${NC}"
        return 0
    else
        echo -e "${RED}✗ FAIL (expected $expected_status, got $status_code)${NC}"
        return 1
    fi
}

# Cleanup on exit
trap stop_server EXIT

# Main test execution
echo "HTTP Server Integration Tests"
echo "============================="

start_server

# Test static files
test_endpoint "GET" "/" "200" "Home page"
test_endpoint "GET" "/css/style.css" "200" "CSS file"
test_endpoint "GET" "/js/script.js" "200" "JavaScript file"

# Test API endpoints
test_endpoint "GET" "/api/time" "200" "Time API"
test_endpoint "GET" "/api/echo?message=test" "200" "Echo API"
test_endpoint "POST" "/api/contact" "200" "Contact form" "name=Test&email=test@example.com&message=Hello"

# Test dynamic pages
test_endpoint "GET" "/status" "200" "Status page"
test_endpoint "GET" "/contact" "200" "Contact page"

# Test authentication
test_endpoint "GET" "/admin" "401" "Admin page (no auth)"

# Test 404 errors
test_endpoint "GET" "/nonexistent" "404" "404 error"

# Test method not allowed
test_endpoint "POST" "/nonexistent" "405" "Method not allowed"

echo
echo -e "${GREEN}Integration tests completed!${NC}"
```

Make it executable and run:
```bash
chmod +x tests/integration/test_server.sh
cd tests/integration
./test_server.sh
```

## Load Testing

### Simple Load Test Script

Create `tests/load/load_test.sh`:

```bash
#!/bin/bash

SERVER_URL="http://localhost:8080"
CONCURRENT_USERS=10
REQUESTS_PER_USER=100
TOTAL_REQUESTS=$((CONCURRENT_USERS * REQUESTS_PER_USER))

echo "Load Testing HTTP Server"
echo "========================"
echo "URL: $SERVER_URL"
echo "Concurrent Users: $CONCURRENT_USERS"
echo "Requests per User: $REQUESTS_PER_USER"
echo "Total Requests: $TOTAL_REQUESTS"
echo

# Function to make requests
make_requests() {
    local user_id=$1
    local start_time=$(date +%s.%N)
    
    for i in $(seq 1 $REQUESTS_PER_USER); do
        curl -s "$SERVER_URL/" > /dev/null
        if [ $? -ne 0 ]; then
            echo "User $user_id: Request $i failed"
        fi
    done
    
    local end_time=$(date +%s.%N)
    local duration=$(echo "$end_time - $start_time" | bc)
    echo "User $user_id completed in $duration seconds"
}

# Start load test
echo "Starting load test..."
start_time=$(date +%s.%N)

# Start concurrent users
for i in $(seq 1 $CONCURRENT_USERS); do
    make_requests $i &
done

# Wait for all users to complete
wait

end_time=$(date +%s.%N)
total_time=$(echo "$end_time - $start_time" | bc)
requests_per_second=$(echo "scale=2; $TOTAL_REQUESTS / $total_time" | bc)

echo
echo "Load test completed!"
echo "Total time: $total_time seconds"
echo "Requests per second: $requests_per_second"
```

### Using Apache Bench (ab)

If you have Apache Bench installed:

```bash
# Test with 10 concurrent connections, 1000 total requests
ab -n 1000 -c 10 http://localhost:8080/

# Test keep-alive connections
ab -n 1000 -c 10 -k http://localhost:8080/

# Test POST requests
ab -n 100 -c 5 -p tests/data/test.json -T application/json http://localhost:8080/api/contact
```

## Security Testing

### Test Directory Traversal

Create `tests/security/security_test.sh`:

```bash
#!/bin/bash

SERVER_URL="http://localhost:8080"

echo "Security Testing"
echo "================"

# Test directory traversal attempts
echo "Testing directory traversal protection..."

test_paths=(
    "../../../etc/passwd"
    "..%2F..%2F..%2Fetc%2Fpasswd"
    "....//....//....//etc/passwd"
    "/etc/passwd"
    "\\..\\..\\..\\windows\\system32\\drivers\\etc\\hosts"
)

for path in "${test_paths[@]}"; do
    echo -n "Testing: $path ... "
    response=$(curl -s -w "%{http_code}" "$SERVER_URL/$path")
    status_code="${response: -3}"
    
    if [ "$status_code" = "404" ] || [ "$status_code" = "403" ]; then
        echo "✓ SAFE (got $status_code)"
    else
        echo "⚠ POTENTIAL ISSUE (got $status_code)"
    fi
done

# Test large request
echo
echo "Testing large request handling..."
large_data=$(python3 -c "print('A' * 2000000)")  # 2MB of data
response=$(curl -s -w "%{http_code}" -X POST -d "$large_data" "$SERVER_URL/api/contact" 2>/dev/null)
status_code="${response: -3}"
echo "Large request test: got status $status_code"

echo
echo "Security tests completed!"
```

## Debugging Techniques

### Using GDB

Compile with debug symbols:
```bash
gcc -g -Wall -Wextra -std=c99 -I include -o server src/*.c
```

Debug with GDB:
```bash
gdb ./server

# In GDB:
(gdb) run
(gdb) break handle_request
(gdb) continue
(gdb) print request->uri
(gdb) step
(gdb) backtrace
```

### Memory Debugging with Valgrind

Install Valgrind and run:
```bash
valgrind --leak-check=full --show-leak-kinds=all ./server
```

### Adding Debug Logging

Add debug macros to your code:

```c
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

// Usage:
DEBUG_PRINT("Parsing request: %s", raw_request);
```

Compile with debug:
```bash
gcc -DDEBUG -g -Wall -Wextra -std=c99 -I include -o server src/*.c
```

## Common Issues and Solutions

### Server Won't Start
- **Port already in use**: Change port or kill existing process
- **Permission denied**: Use port > 1024 or run with sudo
- **Bind failed**: Check if another service is using the port

### Segmentation Faults
- **NULL pointer access**: Check all pointer dereferences
- **Buffer overflows**: Validate array bounds
- **Use after free**: Check memory management

### Memory Leaks
- **Unfreed malloc**: Every malloc needs a corresponding free
- **Strdup leaks**: Remember to free strings created with strdup
- **Response body leaks**: Free response bodies in cleanup

### Performance Issues
- **Blocking I/O**: Consider non-blocking sockets
- **Memory allocation**: Reuse buffers where possible
- **File I/O**: Cache frequently accessed files

## Automated Testing Setup

Create `tests/run_all_tests.sh`:

```bash
#!/bin/bash

echo "Running Complete Test Suite"
echo "=========================="

# Build the server
echo "Building server..."
cd ..
make clean && make
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

cd tests

# Run unit tests
echo
echo "Running unit tests..."
make run_unit_tests

# Run integration tests
echo
echo "Running integration tests..."
cd integration
./test_server.sh
cd ..

# Run security tests
echo
echo "Running security tests..."
cd security
./security_test.sh
cd ..

echo
echo "All tests completed!"
```

## Continuous Testing

For ongoing development, consider setting up:
- Git hooks to run tests before commits
- Automated testing on code changes
- Performance regression testing
- Security scanning

## Best Practices

1. **Test Early and Often**: Write tests as you develop features
2. **Test Edge Cases**: Empty requests, malformed data, large inputs
3. **Test Error Conditions**: Network failures, out of memory, etc.
4. **Use Realistic Data**: Test with real-world HTTP requests
5. **Monitor Performance**: Track response times and memory usage
6. **Security First**: Always test for common vulnerabilities

---

**Previous:** [← Advanced Features](06-advanced-features.md) | **Next:** [Security Guide →](08-security-guide.md)
