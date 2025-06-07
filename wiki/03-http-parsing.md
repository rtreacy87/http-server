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


```
+---------------------------+
|        http.h             |
+---------------------------+
| #define MAX_HEADERS 50    |
| #define MAX_HEADER_SIZE 256|
| #define MAX_URI_SIZE 1024 |
| #define MAX_METHOD_SIZE 16|
| #define MAX_VERSION_SIZE 16|
+---------------------------+
|                           |
| struct http_request_t     |
| +-------------------------+
| | char method[]           |
| | char uri[]              |
| | char version[]          |
| | char headers[][][]      |
| | int header_count        |
| | char* body              |
| | size_t body_length      |
| +-------------------------+
|                           |
| struct http_response_t    |
| +-------------------------+
| | int status_code         |
| | char headers[][][]      |
| | int header_count        |
| | char* body              |
| | size_t body_length      |
| +-------------------------+
|                           |
| Function Declarations     |
| +-------------------------+
| | parse_http_request()    |
| | init_http_request()     |
| | free_http_request()     |
| | init_http_response()    |
| | free_http_response()    |
| | send_http_response()    |
| +-------------------------+
+---------------------------+
```
This diagram shows the structure of `http.h`, which defines the core data structures and functions for HTTP request/response handling:

1. **Constants**: Defines size limits for headers (50 max), header strings (256 chars), URIs (1024 chars), and HTTP methods/versions (16 chars each).

2. **Data Structures**:
   - `http_request_t`: Stores parsed HTTP request components (method, URI, version, headers, body)
   - `http_response_t`: Represents HTTP responses with status code, headers, and body

3. **Function Declarations**: Lists the core functions for:
   - Parsing HTTP requests
   - Initializing/freeing request and response structures
   - Sending HTTP responses

The diagram visually represents how these components are organized in the header file, showing the relationship between the structures and their member variables.

The `.h` extension stands for "header file" in C programming. Header files serve several important purposes:

1. **Declaration vs. Implementation**: They contain declarations (function prototypes, type definitions, constants) while `.c` files contain the actual implementations. This separation helps organize code.

2. **Code Sharing**: They allow multiple `.c` files to share common declarations without duplicating code.

3. **Interface Definition**: They define the public interface of a module or library, specifying what functions and data structures are available to users.

4. **Compilation Efficiency**: They support the "compile once, include many times" approach through include guards (`#ifndef`, `#define`, `#endif`), which prevents multiple inclusions.

5. **Encapsulation**: They help hide implementation details while exposing only necessary interfaces.

In the case of `http.h`, it defines the data structures and function declarations needed for HTTP processing, which can then be included by any `.c` file that needs to work with HTTP requests or responses.

# Declarations vs. Implementations in C

## Examples

### Declaration (in header file)
````c path=include/http.h mode=EXCERPT
// Function declaration (prototype)
int parse_http_request(const char* raw_request, http_request_t* request);

// Type declaration
typedef struct {
    int status_code;
    char headers[MAX_HEADERS][2][MAX_HEADER_SIZE];
    int header_count;
    char* body;
    size_t body_length;
} http_response_t;
````

### Implementation (in source file)
````c path=src/http_parser.c mode=EXCERPT
// Function implementation
int parse_http_request(const char* raw_request, http_request_t* request) {
    init_http_request(request);
    
    // Find the end of headers (double CRLF)
    const char* header_end = strstr(raw_request, "\r\n\r\n");
    if (!header_end) {
        header_end = strstr(raw_request, "\n\n");
        // Implementation details...
    }
    // More implementation code...
    return 0;
}
````

## Why Split Them?

1. **Modularity**: Allows multiple files to use the same functions without duplicating code
2. **Compilation Speed**: Only modified `.c` files need recompilation, not all files that include the header
3. **Information Hiding**: Users only need to know how to use functions (interface), not how they work internally
4. **Maintainability**: Changes to implementation don't require changes to all files using the function
5. **Reduced Coupling**: Source files only depend on interfaces, not implementations of other modules

This separation is a fundamental principle in C programming that supports building larger, more maintainable codebases.

```
+----------------+                 +----------------+
| include/http.h |                 | src/server.c   |
+----------------+                 +----------------+
| #ifndef HTTP_H |                 | #include       |
| #define HTTP_H |                 | "../include/   |
|                |                 |  http.h"       |
| // Declarations|                 |                |
| typedef struct |                 | void           |
| http_request_t;|                 | handle_request |
|                |                 | {              |
| int parse_http_|---------------->|   http_request_t|
| request(...);  |                 |   request;     |
|                |                 |                |
| #endif         |                 |   parse_http_  |
+----------------+                 |   request(...);|
       ^                           |   ...          |
       |                           | }              |
       |                           +----------------+
       |                                   ^
       |                                   |
       |                                   |
+----------------+                         |
| src/http_      |                         |
| parser.c       |                         |
+----------------+                         |
| #include       |                         |
| "../include/   |                         |
|  http.h"       |                         |
|                |                         |
| // Implementation                        |
| int parse_http_|-------------------------+
| request(...) { |
|   // actual    |
|   // code here |
| }              |
+----------------+
```

## What's Happening

1. **Declaration in Header File**:
   - `http.h` contains function prototypes and type definitions
   - Include guards (`#ifndef`, `#define`, `#endif`) prevent multiple inclusions

2. **Implementation in Source File**:
   - `http_parser.c` includes the header file
   - Implements the actual function code that matches the declaration

3. **Usage in Other Files**:
   - `server.c` includes the same header file
   - Can use the declared types and call the functions
   - Compiler knows the function exists but doesn't need implementation details

4. **Compilation Process**:
   - Each `.c` file is compiled separately into object files
   - Compiler verifies function calls match declarations
   - Linker resolves references between object files
   - Final executable contains all implementations

This separation allows multiple files to share the same interface while the implementation can change independently, as long as the function signature remains the same.

Let's break it down with a very simple example:

## Simple Example: A Function to Square a Number

### Declaration (in header file)
````c path=include/math_utils.h mode=EXCERPT
// Declaration in math_utils.h
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

// Function declaration - just tells WHAT the function does
int square(int number);

#endif
````

### Implementation (in source file)
````c path=src/math_utils.c mode=EXCERPT
// Implementation in math_utils.c
#include "../include/math_utils.h"

// Function implementation - defines HOW the function works
int square(int number) {
    return number * number;
}
````

### Usage (in another source file)
````c path=src/main.c mode=EXCERPT
// Usage in main.c
#include <stdio.h>
#include "../include/math_utils.h"

int main() {
    int x = 5;
    int result = square(x);  // We can call the function here
    printf("%d squared is %d\n", x, result);
    return 0;
}
````

## What's Happening

1. In `math_utils.h`, we **declare** the function `square` - we're just saying "there exists a function called `square` that takes an int and returns an int"

2. In `math_utils.c`, we **implement** the function - we define what it actually does (multiplies the number by itself)

3. In `main.c`, we **use** the function - we include the header file to know the function exists, and the compiler trusts that the implementation will be available at link time

When we compile:
1. Each `.c` file is compiled separately
2. The compiler checks that function calls match declarations
3. The linker connects the function call in `main.c` to the actual implementation in `math_utils.c`

This separation means:
- `main.c` only needs to know the function exists and its signature
- If we change how `square` works internally, we only need to modify `math_utils.c`
- Multiple files can use `square` without duplicating code

The header file doesn't actually link to where the function is - it just declares that the function exists. The actual linking happens later during the compilation process. Here's why using header files is better than directly including `.c` files:

1. **Compilation Efficiency**: 
   - If you include `math_utils.c` directly, its code gets compiled every time any file includes it
   - With headers, `math_utils.c` is compiled only once, saving compilation time

2. **Avoiding Multiple Definitions**:
   - Including a `.c` file multiple times would cause "multiple definition" errors
   - The same function would be defined in every file that includes it

3. **Separation of Interface and Implementation**:
   - Headers provide a clean interface without exposing implementation details
   - Users only need to know how to use functions, not how they work internally

4. **Incremental Compilation**:
   - When you change implementation in `math_utils.c`, only that file needs recompilation
   - If you included `.c` files directly, changing one file would require recompiling everything

5. **Encapsulation**:
   - Implementation details can be hidden from users of your code
   - You can change implementation without affecting code that uses your functions

This separation is a fundamental principle in C programming that enables building larger, more maintainable codebases with clearer boundaries between components.
**benifites of the header file system**:

1. **Separate compilation**: Each `.c` file is compiled independently into an object file (`.o`).

2. **Minimal recompilation**: When you change code in one file, only that file needs to be recompiled, not everything that uses it.

3. **Efficient linking**: The linker connects function calls to their implementations using the information from the declarations.

This is especially important for large projects where:
- Complete recompilation could take hours
- Teams work on different parts of the codebase simultaneously
- Libraries are developed separately from applications that use them

The header file system creates a clean separation between "what a function does" (interface) and "how it does it" (implementation), allowing for more efficient compilation and better code organization.

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
**http.h Explained**
```c
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#define MAX_HEADERS 50
#define MAX_HEADER_SIZE 256
#define MAX_URI_SIZE 1024
#define MAX_METHOD_SIZE 16
#define MAX_VERSION_SIZE 16
```
Let's break down each line in detail:

1. `#ifndef HTTP_H`
   - This is an "include guard" that starts a conditional compilation block
   - It checks if the symbol `HTTP_H` is not defined yet
   - Purpose: Prevents the header from being included multiple times in the same compilation unit

2. `#define HTTP_H`
   - Defines the symbol `HTTP_H`
   - After this line, the symbol is defined, so future `#ifndef HTTP_H` checks will fail
   - Works with line 1 to create the include guard

3. `#include <stddef.h>`
   - Includes the standard C library header `stddef.h`
   - This header provides `size_t` type definition, which is used for sizes and counts
   - Uses angle brackets `<>` to search in standard include directories

4. `#define MAX_HEADERS 50`
   - Creates a constant named `MAX_HEADERS` with value 50
   - Sets the maximum number of HTTP headers the server will process
   - Using a #define makes it easy to change this limit in one place

5. `#define MAX_HEADER_SIZE 256`
   - Creates a constant for maximum header string length (256 characters)
   - Limits how long each header name or value can be
   - Prevents buffer overflows by establishing clear boundaries

6. `#define MAX_URI_SIZE 1024`
   - Sets maximum URI (URL path) length to 1024 characters
   - URIs are typically longer than headers, hence the larger size
   - Security measure to prevent extremely long paths

7. `#define MAX_METHOD_SIZE 16`
   - Limits HTTP method names (GET, POST, etc.) to 16 characters
   - Standard methods are short (3-7 chars), so this is generous
   - Saves memory by not allocating excessive space

8. `#define MAX_VERSION_SIZE 16`
   - Sets maximum HTTP version string length (e.g., "HTTP/1.1")
   - 16 characters is more than enough for current and future versions
   - Consistent with other size limitations

Together, these lines set up the foundation for the HTTP header file by:
- Ensuring the header is only processed once
- Including necessary dependencies
- Defining size constraints for security and memory efficiency

**Aside:What is a Header File?**

In HTTP, a header is a key-value pair that provides additional information about an HTTP request or response. Headers are part of the metadata that accompanies the actual content (body) of HTTP messages.

Key characteristics of HTTP headers:

1. **Format**: Each header consists of a name followed by a colon, then a value (e.g., `Content-Type: text/html`)

2. **Purpose**: Headers convey important information such as:
   - Content type and length
   - Authentication credentials
   - Caching directives
   - Cookie information
   - Server information
   - Compression methods

3. **Common Examples**:
   - `Content-Type`: Specifies the media type of the resource
   - `Content-Length`: Indicates the size of the body in bytes
   - `User-Agent`: Identifies the client software
   - `Authorization`: Contains authentication credentials
   - `Cookie`: Contains stored HTTP cookies

4. **Implementation**: In the HTTP server code we're examining, headers are stored in a 3D array:
   ```c
   char headers[MAX_HEADERS][2][MAX_HEADER_SIZE];
   ```
   This allows for up to 50 headers, each with a name (index 0) and value (index 1), with each string limited to 256 characters.

Headers are essential for proper HTTP communication, allowing clients and servers to exchange metadata necessary for correctly processing requests and responses.
**What is a URI?**

A URI (Uniform Resource Identifier) is a string that identifies a resource on the web. In the context of HTTP servers:

1. **Definition**: A URI is the path portion of a URL (Uniform Resource Locator) that specifies what resource the client is requesting from the server.

2. **Format**: In an HTTP request, the URI appears in the request line:
   ```
   GET /index.html HTTP/1.1
   ```
   Here, `/index.html` is the URI.

3. **Examples**:
   - `/` - The root or home page
   - `/about.html` - A specific page
   - `/images/logo.png` - An image file
   - `/api/users/123` - An API endpoint with a parameter
   - `/search?q=keyword&page=2` - A path with query parameters

4. **Security Concerns**: URIs need validation to prevent:
   - Directory traversal attacks (using `../` to access files outside web root)
   - Malicious character sequences
   - Excessively long paths that could cause buffer overflows

5. **Implementation**: In the HTTP server code, URIs are limited to 1024 characters:
   ```c
   #define MAX_URI_SIZE 1024
   ```
   This provides ample space for most legitimate URIs while preventing extremely long paths.

URIs are central to how web servers work - they're the primary mechanism by which clients tell servers what resource they want to access.
```
HTTP REQUEST ENCAPSULATION & LIMITS
===================================

CLIENT REQUEST                                SERVER PROCESSING
+----------------------------------+          +----------------------------------+
| GET /products/123 HTTP/1.1      |          | METHOD: "GET"                    |
| Host: example.com               |          | #define MAX_METHOD_SIZE 16       |
| User-Agent: Mozilla/5.0         |          | char method[16];                 |
| Accept: text/html               |          |                                  |
| Cookie: session=abc123          |          +----------------------------------+
| ...                             |          |                                  |
+----------------------------------+          | URI: "/products/123"            |
                                             | #define MAX_URI_SIZE 1024        |
                                             | char uri[1024];                  |
                                             |                                  |
                                             +----------------------------------+
                                             |                                  |
                                             | VERSION: "HTTP/1.1"              |
                                             | #define MAX_VERSION_SIZE 16      |
                                             | char version[16];                |
                                             |                                  |
                                             +----------------------------------+
                                             |                                  |
                                             | HEADERS:                         |
                                             | #define MAX_HEADERS 50           |
                                             | #define MAX_HEADER_SIZE 256      |
                                             | char headers[50][2][256];        |
                                             |                                  |
                                             | [0][0]: "Host"                   |
                                             | [0][1]: "example.com"            |
                                             |                                  |
                                             | [1][0]: "User-Agent"             |
                                             | [1][1]: "Mozilla/5.0"            |
                                             |                                  |
                                             | [2][0]: "Accept"                 |
                                             | [2][1]: "text/html"              |
                                             |                                  |
                                             | [3][0]: "Cookie"                 |
                                             | [3][1]: "session=abc123"         |
                                             |                                  |
                                             | header_count: 4                  |
                                             |                                  |
                                             +----------------------------------+
                                             |                                  |
                                             | BODY: (empty for GET)            |
                                             | char* body;                      |
                                             | size_t body_length;              |
                                             |                                  |
                                             +----------------------------------+

SECURITY CONSTRAINTS:
- Method limited to 16 chars (prevents buffer overflow attacks via oversized methods)
- URI limited to 1024 chars (prevents extremely long paths)
- Maximum 50 headers (prevents DoS via excessive headers)
- Each header name/value limited to 256 chars (prevents memory exhaustion)
- Version string limited to 16 chars (sufficient for all HTTP versions)
```

## Security Considerations

Attackers can attempt DoS (Denial of Service) attacks using HTTP headers in several ways:

1. **Excessive Number of Headers**:
   - Without the `MAX_HEADERS` limit (50), an attacker could send thousands of headers
   - Each header requires memory allocation and processing time
   - Example: Sending 10,000+ custom headers in a single request

2. **Extremely Long Header Values**:
   - Without the `MAX_HEADER_SIZE` limit (256), headers could be megabytes in size
   - Example: A `Cookie` header with 100MB of random data
   - Server would need to allocate memory and process this oversized data

3. **Header Parsing Complexity**:
   - Some headers require complex parsing (dates, encoding information)
   - Malformed headers can cause excessive CPU usage during parsing
   - Example: Specially crafted `Accept-Encoding` headers that trigger worst-case parsing scenarios

4. **Memory Exhaustion**:
   - Each connection requires memory to store headers
   - With unlimited headers, attackers could exhaust server memory
   - Example: Opening thousands of connections, each with maximum-sized headers

5. **Real-World Example**: The "Slowloris" attack sends HTTP headers very slowly, one at a time, keeping connections open for long periods to exhaust connection pools.

The limits in the code (`MAX_HEADERS=50`, `MAX_HEADER_SIZE=256`) create predictable memory usage patterns and prevent these attacks by:
- Limiting total memory allocation per request
- Ensuring parsing completes in reasonable time
- Rejecting malicious requests that exceed these limits

This is why proper input validation and limits are essential security measures in HTTP servers.

**DDOS via Headers and Protection**

1. **Headers are key-value pairs**: HTTP headers are simply metadata in the format `Name: Value`. They provide information about the request or response (like content type, length, cookies, etc.).

2. **DoS protection through limits**: The `MAX_HEADERS` and `MAX_HEADER_SIZE` constants protect against two specific attacks:

   - **Quantity attack**: Without the `MAX_HEADERS` limit, an attacker could send a single request with millions of custom headers:
     ```
     GET / HTTP/1.1
     X-Custom-1: value
     X-Custom-2: value
     ...
     X-Custom-1000000: value
     ```
     Each header requires memory allocation and processing time.

   - **Size attack**: Without the `MAX_HEADER_SIZE` limit, an attacker could send extremely large header values:
     ```
     GET / HTTP/1.1
     Cookie: [100MB of random data]
     ```

These limits create predictable memory usage patterns. When a request exceeds these limits, the server can simply reject it rather than trying to allocate excessive memory or spending too much time parsing.

It's a simple but effective defense against a class of attacks that have successfully taken down servers in the past.

The two typedef structures in the selected code define the core data models for HTTP communication:

```c
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
```

Separating them is important for several reasons:

1. **Different Purposes**: 
   - `http_request_t` represents client requests TO the server
   - `http_response_t` represents server responses TO the client
   - They have fundamentally different roles in HTTP communication

2. **Different Fields**:
   - Requests have method, URI, and version fields
   - Responses have a status code instead
   - These differences reflect the HTTP protocol specification

3. **Single Responsibility Principle**:
   - Each structure handles one specific concept
   - Makes code more maintainable and easier to understand

4. **Modularity**:
   - Functions can be designed to work with just requests or just responses
   - Allows for cleaner function signatures and better encapsulation

5. **Future Extensibility**:
   - Each structure can evolve independently as needed
   - New request features don't need to affect response handling

This separation mirrors the HTTP protocol itself, which clearly distinguishes between requests and responses. It creates a cleaner mental model for developers working with the code and makes the implementation more closely match the problem domain.

The difference between `char` and `char*` is fundamental in C:

1. **`char`**: 
   - Represents a single character
   - Occupies 1 byte of memory
   - Example: `char c = 'A';`

2. **`char*`** (pointer to char):
   - Represents a memory address that points to the first character of a sequence
   - The pointer itself typically occupies 4 or 8 bytes (depending on architecture)
   - Can point to a single character or the first character of a string
   - Example: `char* str = "Hello";`

In the HTTP structures:

```c
// Fixed-size character arrays (space pre-allocated)
char method[MAX_METHOD_SIZE];    // Space for up to 16 characters
char uri[MAX_URI_SIZE];          // Space for up to 1024 characters

// Pointer to dynamically allocated memory
char* body;                      // Just a pointer, memory allocated elsewhere
```

The key differences in usage:

1. **Fixed arrays** (`char method[16]`):
   - Memory is allocated at compile time
   - Size is fixed and cannot change
   - Memory is part of the structure itself
   - Good for data with known maximum size

2. **Pointers** (`char* body`):
   - Only the pointer is part of the structure
   - Memory must be allocated separately (usually with `malloc`)
   - Size can be dynamic
   - Must be explicitly freed to avoid memory leaks
   - Good for data with unknown or variable size

In the HTTP structures, the body uses a pointer because request/response bodies can be of any size, while methods, URIs, and headers have reasonable maximum sizes and use fixed arrays.

The line `char headers[MAX_HEADERS][2][MAX_HEADER_SIZE];` defines a 3-dimensional array for storing HTTP headers. Let's break it down:

```
char headers[MAX_HEADERS][2][MAX_HEADER_SIZE];
           |            |  |
           |            |  +-- Each string can be up to 256 characters
           |            |
           |            +---- Two strings per header (name and value)
           |
           +---- Space for up to 50 headers
```

This means:

1. **First dimension** `[MAX_HEADERS]` (50):
   - Allows storing up to 50 different headers
   - Each index (0-49) represents one complete header

2. **Second dimension** `[2]`:
   - Each header has exactly 2 parts:
   - `headers[i][0]` stores the header name (e.g., "Content-Type")
   - `headers[i][1]` stores the header value (e.g., "text/html")

3. **Third dimension** `[MAX_HEADER_SIZE]` (256):
   - Each header name or value can be up to 256 characters long
   - This is the actual character array for the strings

In usage:
- `headers[0][0]` might contain "Host"
- `headers[0][1]` might contain "example.com"
- `headers[1][0]` might contain "User-Agent"
- `headers[1][1]` might contain "Mozilla/5.0"

The `header_count` field in the structure keeps track of how many headers are actually being used (0 to 50).

This approach efficiently organizes header data while enforcing size limits for security and memory management.

`size_t` is a special unsigned integer type in C that:

1. **Purpose**: Represents sizes and counts of objects in memory
   - Used for array indices, memory allocation sizes, string lengths, etc.
   - Always non-negative (unsigned)

2. **Size Guarantee**: Guaranteed to be large enough to represent the maximum size of any object in memory
   - On 32-bit systems: typically 4 bytes (0 to ~4.2 billion)
   - On 64-bit systems: typically 8 bytes (0 to ~18.4 quintillion)

3. **Portability**: Makes code more portable across different platforms
   - Automatically adjusts to the appropriate size for the architecture
   - Prevents overflow issues when dealing with large memory objects

4. **Standard Usage**: Used consistently throughout the C standard library
   - `malloc(size_t size)`
   - `strlen()` returns `size_t`
   - Array indexing functions use `size_t`

In the HTTP structures, `size_t body_length` indicates:
- The length of the body data in bytes
- Can handle bodies of any size the system can allocate
- Consistent with standard C functions that will interact with this field

Using `size_t` instead of `int` is important because:
- It avoids negative values (which make no sense for lengths)
- It can represent larger values on 64-bit systems
- It matches what standard library functions expect

1. **Custom Types (typedef struct)**:
   - `http_request_t` and `http_response_t` are custom data types created to represent HTTP requests and responses
   - They package all the relevant data fields into coherent structures
   - These types make function signatures cleaner and more meaningful

2. **Function Declarations**:
   - The declarations define the interface for HTTP processing functions
   - They specify what functions exist, their parameters, and return types
   - But they don't include the actual implementation code

The specific functions declared cover the core operations needed for HTTP communication:

- `parse_http_request`: Converts raw HTTP request text into the structured `http_request_t`
- `init_http_request`/`init_http_response`: Initialize the structures with default values
- `free_http_request`/`free_http_response`: Clean up any dynamically allocated memory
- `send_http_response`: Transmits an HTTP response to a client

Together, these types and function declarations create a complete API for handling HTTP requests and responses. Any C file that includes this header can work with HTTP data in a structured way without needing to know the implementation details.

You've got it almost exactly right! Let's refine a few details:

1. **Return Types**:
   - `int parse_http_request()` and `int send_http_response()`: Return integers, typically 0 for success and non-zero values for different error conditions (not just 1)
   - `void init_http_request()`, etc.: Return nothing, as you correctly noted

2. **Parameter Types**:
   - `const char* raw_request`: Points to a string in memory that shouldn't be modified (the `const` prevents modification)
   - `http_request_t* request` and `http_response_t* response`: These are pointers to structures that the functions will modify
   - `int client_fd`: An integer representing a file descriptor for the client socket

3. **Parameter Purposes**:
   - Pointers allow functions to modify the original data rather than working on copies
   - The `const` qualifier on `raw_request` indicates the function will read but not modify this data
   - The absence of `const` on other pointers indicates the function will modify those structures

4. **Typical Usage Pattern**:
   - Functions that initialize or modify data structures take pointers to those structures
   - Functions that only need to read data might take `const` pointers
   - Functions that need to return complex data often modify a structure passed by pointer rather than returning the data directly

Your understanding of the overall pattern is spot on - these declarations establish a clear API for HTTP processing with appropriate return types and parameters for each operation.




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
