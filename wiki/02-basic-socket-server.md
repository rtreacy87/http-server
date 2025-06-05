# Phase 1: Basic Socket Server

## What We're Building

In this phase, we'll create a simple TCP server that:
- Listens on port 8080
- Accepts client connections
- Reads data from clients
- Sends back a simple message
- Handles one client at a time

This is the foundation for our HTTP server!

## Understanding Sockets

Think of a socket like a telephone:
- **Server socket**: Like a phone number that clients can call
- **Client socket**: Like making a phone call to that number
- **Connection**: The conversation between client and server

```
                                 SERVER.C FLOW
                                 =============

+------------------+     +-------------------+     +-------------------+
| INITIALIZATION   |     | SETUP             |     | CONNECTION LOOP   |
+------------------+     +-------------------+     +-------------------+
| - Include headers|     | 1. Create socket  |     |    +-----------+  |
| - Define PORT    |     |    server_fd      |     |    | Wait for  |  |
| - Define BUFFER  |     | 2. Configure addr |     |    | connection|  |
| - Declare vars   |     | 3. Bind socket    |     |    +-----------+  |
+------------------+     | 4. Listen         |     |          |        |
                         +-------------------+     |          v        |
                                                   |    +-----------+  |
                                                   |    | Accept    |  |
                                                   |    | client_fd |  |
                                                   |    +-----------+  |
                                                   |          |        |
                                                   |          v        |
                                                   |    +-----------+  |
                                                   |    | Read data |  |
                                                   |    | from      |  |
                                                   |    | client    |  |
                                                   |    +-----------+  |
                                                   |          |        |
                                                   |          v        |
                                                   |    +-----------+  |
                                                   |    | Send      |  |
                                                   |    | response  |  |
                                                   |    +-----------+  |
                                                   |          |        |
                                                   |          v        |
                                                   |    +-----------+  |
                                                   |    | Close     |  |
                                                   |    | client_fd |  |
                                                   |    +-----------+  |
                                                   |          |        |
                                                   |          v        |
                                                   |    (Loop back)    |
                                                   +-------------------+

NETWORK FLOW:
                                                  
  +--------+                                    +--------+
  | CLIENT |                                    | SERVER |
  +--------+                                    +--------+
      |                                             |
      |        1. Connection Request                |
      | -----------------------------------------> |
      |                                             |
      |        2. Connection Accepted               |
      | <----------------------------------------- |
      |                                             |
      |        3. Send Data                         |
      | -----------------------------------------> |
      |                                             |
      |        4. Send Response                     |
      | <----------------------------------------- |
      |                                             |
      |        5. Connection Closed                 |
      | <----------------------------------------- |
      |                                             |
```

This diagram shows:
1. The program initialization and setup phases
2. The main connection loop that handles clients
3. The network flow between client and server

The server creates a socket, binds it to port 8080, and listens for connections. When a client connects, the server accepts the connection, reads data, sends a response, and closes the client connection before looping back to wait for the next client.

**Q: What is the difference between a socket and a port?**

The difference between a socket and a port is an important distinction:

**Port:**
- A port is a numerical identifier (0-65535) that represents a specific communication endpoint on a computer
- Ports are part of the transport layer (TCP/UDP) addressing system
- They allow a single computer with one IP address to run multiple network services
- Ports are like different doors into a building (the computer)
- Examples: HTTP (80), HTTPS (443), SSH (22)
- Ports are a concept, not an actual object in code

**Socket:**
- A socket is a software object that represents one endpoint of a network connection
- It combines an IP address and a port number, plus the protocol being used
- Sockets are the programming interface that applications use to communicate over a network
- In your code, a socket is represented by a file descriptor
- Sockets can be created, connected, read from, written to, and closed
- A socket is an actual programming construct you interact with

To use an analogy:
- If your computer is a building, ports are like numbered doors
- A socket is like having an open communication channel through a specific door
- Multiple sockets can use the same port (though not simultaneously for listening)

In your server code:
- You specify port 8080 as where your server will listen
- The socket (`server_fd`) is the actual communication endpoint that's bound to that port
- When a client connects, a new socket (`client_fd`) is created for that specific connection

So ports are more like addresses or identifiers, while sockets are the actual mechanisms (objects) that your program uses to communicate through those ports.

Using the phone analogy:

- A **port** is like your phone number. It's just an identifier that allows others to reach your computer, but it doesn't do anything by itself.

- A **socket** is like the actual phone connection that's established when you set up a phone line. It's the real communication channel that allows data to flow back and forth.

To extend this analogy further:

1. **Creating a socket** (`socket()`) is like installing a phone in your house.

2. **Binding to a port** (`bind()`) is like registering a specific phone number for your phone.

3. **Listening** (`listen()`) is like turning the ringer on so you can hear when someone calls.

4. **Accepting a connection** (`accept()`) is like answering the phone when it rings.

5. **The client socket** (`client_fd`) is like the dedicated line between you and the caller during a conversation.

6. **Reading and writing** (`read()/write()`) is like speaking and listening during the phone call.

7. **Closing the connection** (`close()`) is like hanging up the phone.

And just like how multiple people can call the same phone number (but not simultaneously unless you have call waiting), multiple clients can connect to the same port, but each gets their own socket (connection).


## Step 1: Create the Basic Server Structure

Create `src/server.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    printf("Starting server on port %d...\n", PORT);
    
    // Step 1: Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Step 2: Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Step 3: Bind socket to address
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }
    
    // Step 4: Listen for connections
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
    
    printf("Server listening on port %d\n", PORT);
    
    // Step 5: Accept and handle connections
    while (1) {
        printf("Waiting for connection...\n");
        
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("Client connected from %s\n", inet_ntoa(client_addr.sin_addr));
        
        // Read data from client
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Received: %s\n", buffer);
            
            // Send response
            const char* response = "Hello from server!\n";
            write(client_fd, response, strlen(response));
        }
        
        // Close client connection
        close(client_fd);
        printf("Client disconnected\n");
    }
    
    close(server_fd);
    return 0;
}
```

## Step 2: Understanding the Code

Let's break down what each part does:

A buffer is a temporary storage area in memory used to hold data while it's being transferred from one place to another. Think of it like a bucket that temporarily holds water when you're moving it between containers.

In programming, buffers are particularly important for:

1. **Input/Output operations**: When reading from or writing to devices, files, or networks
2. **Data processing**: When you need to collect data before processing it all at once
3. **Performance optimization**: To reduce the number of system calls or hardware interactions

In your socket server code, you see buffers in action:

```c
#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];  // Creates a buffer of 1024 bytes

// Later in the code:
int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
```

Here's what this buffer does:

1. It allocates 1024 bytes of memory (an array of characters)
2. When a client sends data, the `read()` function places that data into this buffer
3. The program can then access and process the data from the buffer
4. The buffer is reused for each new client connection

Key characteristics of buffers:

- They have a fixed size (like `BUFFER_SIZE` in your code)
- They can overflow if more data is written than they can hold (hence the `-1` in the read call to leave room for a null terminator)
- They're typically implemented as arrays in memory
- They act as an intermediary between different parts of a system

In networking, buffers are crucial because:
- Network data arrives in chunks or packets
- Data might arrive faster than it can be processed
- The program needs a place to store incoming data until it's ready to handle it

The buffer in your code is essentially a temporary holding area for the data sent by clients before your server processes and responds to it.



### Socket Creation
```c
server_fd = socket(AF_INET, SOCK_STREAM, 0);
```
- `AF_INET`: Use IPv4
- `SOCK_STREAM`: Use TCP (reliable, ordered data)
- Returns a file descriptor (like a handle to the socket)

- `AF_INET`: Use IPv4 (Internet Protocol version 4)
  - Other options include `AF_INET6` (IPv6), `AF_UNIX` (Unix domain sockets for local communication)
  - See `man 2 socket` or socket.h documentation for all address families
- `SOCK_STREAM`: Use TCP (reliable, ordered data)
  - Alternative: `SOCK_DGRAM` for UDP (connectionless, message-oriented)
  - `SOCK_RAW` for direct network protocol access
- Returns a file descriptor (like a handle to the socket)
  - Returns a non-negative integer on success (typically 3+ as 0,1,2 are stdin/stdout/stderr)
  - Returns -1 on error
  - Not limited to 0 or 1; each open file/socket gets a unique descriptor
- The socket() function always takes 3 parameters:
  - domain (address family)
  - type (socket type)
  - protocol (usually 0 for default protocol)

### Address Configuration
```c
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = INADDR_ANY;
server_addr.sin_port = htons(PORT);
```
- `sin_family`: IPv4
- `INADDR_ANY`: Listen on all network interfaces
- `htons(PORT)`: Convert port to network byte order

Line-by-line explanation:

The `memset` line is initializing the memory for the `server_addr` structure. Let me break it down:

```c
memset(&server_addr, 0, sizeof(server_addr));
```

- `memset` is a C library function that fills a block of memory with a specified value. It stands for "memory set".

- `&server_addr` is the address of the `server_addr` variable. The `&` is the "address-of" operator in C, which returns the memory address where the variable is stored. This tells `memset` where to start filling memory.

- `0` is the value to fill the memory with. In this case, we're setting every byte to zero (essentially clearing the structure).

- `sizeof(server_addr)` calculates the total size in bytes of the `server_addr` structure. This tells `memset` how many bytes to fill.

So the complete line means: "Fill the entire memory block occupied by the `server_addr` structure with zeros." This is a common practice in C to ensure all fields in a structure start with known values (zeros) before you set specific fields.

This initialization is important because:
1. It clears any garbage values that might be in memory
2. It sets all fields to zero/NULL by default
3. It ensures fields we don't explicitly set later (like `sin_zero` padding) are properly initialized

Without this initialization, the structure might contain random data from whatever was previously in that memory location, which could cause unpredictable behavior.

`server_addr` is a variable, but it's not a simple variable like an integer or character - it's a structure variable of type `struct sockaddr_in`. Structures in C are composite data types that contain multiple fields of different types.

The `sizeof` operator in C determines the size (in bytes) of its operand at compile time. When you use `sizeof(server_addr)`, it calculates the total memory size needed to store all fields in the `struct sockaddr_in` structure.

For example, a typical `struct sockaddr_in` might contain:
- `sin_family`: 2 bytes
- `sin_port`: 2 bytes
- `sin_addr`: 4 bytes
- `sin_zero`: 8 bytes

So `sizeof(server_addr)` would return 16 bytes (the sum of all field sizes).

The `memset` function then uses this size to know exactly how many bytes to fill with zeros, ensuring the entire structure is initialized properly.

This is different from creating or allocating new memory - the structure variable `server_addr` was already declared earlier in the code. The `memset` call is just initializing the memory that was already allocated for this variable when it was declared.


- `server_addr.sin_family = AF_INET;`
  - `sin_family` specifies the address family/protocol family to use
  - `AF_INET` selects IPv4 addressing (Address Family: Internet)
  - This field is required and must match the first parameter used in the socket() call

- `server_addr.sin_addr.s_addr = INADDR_ANY;`
  - `sin_addr` is a structure containing the IP address
  - `s_addr` is the actual 32-bit IPv4 address in network byte order
  - `INADDR_ANY` (value 0.0.0.0) tells the system to bind to all available network interfaces
  - This allows the server to accept connections to any of the host's IP addresses

- `server_addr.sin_port = htons(PORT);`
  - `sin_port` specifies which port number to use
  - `htons()` converts the port number from host byte order to network byte order (big-endian)
  - This conversion is necessary because different computer architectures store multi-byte values differently

Summary: These three lines configure the socket address structure with the essential information needed for binding: the protocol family (IPv4), which network interfaces to use (all available ones), and which port to listen on (8080, converted to network byte order). This tells the operating system exactly where and how your server should accept incoming connections.

The "sin" in `sin_family`, `sin_addr`, and `sin_port` stands for "socket internet" or "sockaddr_in" - it's a prefix used in the fields of the `struct sockaddr_in` structure.

This naming convention indicates these are members of the IPv4 socket address structure. The structure is defined in the socket API as:

```c
struct sockaddr_in {
    sa_family_t    sin_family;    /* Address family (AF_INET) */
    in_port_t      sin_port;      /* Port number */
    struct in_addr sin_addr;      /* Internet address */
    char           sin_zero[8];   /* Padding */
};
```

The prefix helps distinguish these fields from other socket address structures like `sockaddr_in6` (for IPv6, which uses `sin6_` prefix) or `sockaddr_un` (for Unix domain sockets, which uses `sun_` prefix).

This naming pattern is part of the Berkeley sockets API convention, which has been widely adopted across operating systems.


### Binding
```c
bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))
```
- Associates the socket with the address and port
- Like claiming a phone number

````c
if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(server_fd);
    exit(1);
}
````

Line-by-line explanation:

1. `if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {`
   - The `bind()` function associates the socket with a specific address and port
   - `server_fd` is the socket file descriptor created earlier
   - `(struct sockaddr*)&server_addr` casts our IPv4 address structure to the generic socket address type
   - `sizeof(server_addr)` tells bind how large the address structure is
   - `< 0` checks if bind returned a negative value, which indicates an error
   - The entire line means: "If binding the socket to our address fails..."

    ````
                    MEMORY DIAGRAM: POINTERS VS ADDRESSES
                    ====================================

        VARIABLE                     MEMORY                      POINTER
        --------                     ------                      -------

                                  +----------+
                                  |          |
                                  |  Value   |                 +--------+
        int num = 42;             |    42    |                 |        |
                                  |          |                 |        |
                                  +----------+                 |        |
                                  | Address  |                 |        |
                                  | 0x1000   |                 |        |
                                  +----------+                 |        |
                                       ▲                       |        |
                                       |                       |        |
                                       |                       |        |
                                       |                       |        |
        int *ptr = &num;               |                       | 0x1000 |
        (ptr is a pointer              |                       |        |
         that stores the               +------------------------+        |
         address of num)                                       |        |
                                                               +--------+

        KEY CONCEPTS:
        ------------
        
        1. ADDRESS (&):
           - Every variable in memory has a unique address (e.g., 0x1000)
           - The address-of operator (&) returns this memory location
           - In &num, the & operator gives us the address where num is stored
           - Addresses are just numbers representing memory locations
        
        2. POINTER (*):
           - A pointer is a variable that STORES an address
           - It "points to" another variable by holding its address
           - Pointers have their own memory location and address
           - The * in declaration (int *ptr) means "ptr is a pointer to int"
        
        3. DEREFERENCING (*):
           - Using * on a pointer accesses the value at the address it points to
           - *ptr would give us 42 (the value at address 0x1000)
           - The * operator here means "give me what this pointer points to"
    ````

    The key difference between addresses and pointers:

    - An **address** is simply a number that represents a location in memory. Every variable has an address. You get the address of a variable using the `&` operator (like `&server_addr`).

    - A **pointer** is a variable that stores an address. Pointers are designed to "point to" other variables by storing their memory addresses. A pointer has its own address too.

    In the context of your socket code:
    - `server_addr` is a structure variable with its own address in memory
    - `&server_addr` gives the memory address where this structure is stored
    - When you pass `&server_addr` to functions like `memset()` or `bind()`, you're telling them where to find the structure in memory

    **Further Explanation of the Cast**

    The cast `(struct sockaddr*)&server_addr` doesn't change the address - it just tells the compiler to treat that address as pointing to a different type of structure.

    The `(struct sockaddr*)` in the code is not dereferencing - it's actually doing a type cast.

    Let me explain:

    ```c
    (struct sockaddr*)&server_addr
    ```

    This expression has two parts:
    1. `&server_addr` - Gets the memory address of the `server_addr` variable (which is of type `struct sockaddr_in`)
    2. `(struct sockaddr*)` - Casts (converts) this address to be a pointer to a different type: `struct sockaddr`

    The `struct sockaddr` is a generic socket address structure that can represent addresses for different protocol families. The `bind()` function expects its second parameter to be a pointer to this generic type, not the specific IPv4 type (`struct sockaddr_in`) that we're actually using.

    So this cast is telling the compiler: "I know this is actually a pointer to a `sockaddr_in` structure, but please treat it as a pointer to the more generic `sockaddr` structure for this function call."

    This is necessary because the socket API was designed to work with multiple address families (IPv4, IPv6, Unix domain sockets, etc.), so it uses a generic type in its interface, and we have to cast our specific address type to match what the function expects.

    It's not dereferencing anything - it's just changing how the compiler interprets the type of the address we're passing to the function.

    **Note:Pointers and Addresses**

    ```c
    // Assuming:
    int num = 42;
    int *ptr = &num;

    // To print the address stored in ptr (0x1000):
    printf("%p", ptr);  // Prints: 0x1000

    // To print the address of num (also 0x1000):
    printf("%p", &num); // Prints: 0x1000

    // To print the value that ptr points to (42):
    printf("%d", *ptr); // Prints: 42
    ```

    You need to use the correct format specifier:
    - `%p` for pointers/addresses
    - `%d` for integers

    So yes, your understanding is correct:
    - `ptr` contains the value 0x1000 (an address)
    - `&num` evaluates to 0x1000 (the same address)
    - `*ptr` dereferences the pointer to get the value 42

    This is why in your socket code, passing `&server_addr` to functions gives them the memory address where they can find or modify the structure's data.

    A socket file descriptor is a non-negative integer that serves as a handle or reference to a socket in your program. It's essentially an identifier that the operating system assigns to represent the socket you've created.

    Here's why socket file descriptors are important:

    1. **Resource Identification**: The file descriptor uniquely identifies your socket among all open files and sockets in your process. The operating system uses this number to track which socket you're referring to when you make system calls.

    2. **Unix File Abstraction**: In Unix/Linux systems, sockets follow the "everything is a file" philosophy. The socket file descriptor works similarly to file descriptors for regular files, allowing you to use common I/O operations like `read()` and `write()`.

    3. **Resource Management**: Each process has a limited number of file descriptors available. You need to properly manage these by closing sockets when you're done with them to avoid resource leaks.

    4. **I/O Operations**: You use the file descriptor in subsequent function calls to perform operations on the socket:
       - `bind()`: Associate the socket with an address
       - `listen()`: Mark the socket as passive for accepting connections
       - `accept()`: Accept incoming connections
       - `read()/write()`: Transfer data
       - `close()`: Release the socket

    5. **Multiplexing**: Functions like `select()`, `poll()`, and `epoll()` use file descriptors to monitor multiple sockets simultaneously for activity.

    In your server code, you're using two important file descriptors:
    - `server_fd`: The listening socket that accepts new connections
    - `client_fd`: The communication socket for an individual client connection

    Without proper management of these descriptors (creating, using, and closing them correctly), your server would fail to function properly and could leak system resources.

    
   
2. `perror("Bind failed");`
   - `perror()` prints a descriptive error message to stderr
   - It includes the string "Bind failed" followed by the specific error from the system
   - This helps diagnose what went wrong (e.g., "Bind failed: Address already in use")

3. `close(server_fd);`
   - Closes the socket file descriptor to release the resource
   - This is proper cleanup before exiting due to the error

4. `exit(1);`
   - Terminates the program with exit code 1
   - Non-zero exit codes indicate abnormal termination
   - This prevents the program from continuing after a critical failure

This code doesn't create the connection - it's actually binding the socket to a specific address and port. Let me clarify the sequence:

1. **Socket creation** happens earlier in the code with `socket()` - this creates the socket file descriptor `server_fd`

2. **Binding** is what this code is doing - it associates the socket with a specific IP address and port number

3. **Listening** would come after this code with `listen()` - this marks the socket as passive, ready to accept connections

4. **Accepting connections** would happen in a later part with `accept()` - this is where actual client connections are established

The code you're looking at is just the binding step, and it's checking if that step failed. If binding fails (returns a negative value), it prints an error message, cleans up by closing the socket, and exits the program.

Common reasons binding might fail include:
- The port is already in use by another program
- You don't have permission to use that port (ports below 1024 require root/admin privileges)
- The address format is incorrect

So you're correct - this code isn't creating a connection. It's preparing the socket to be able to accept connections later by binding it to an address and port, and handling any errors that might occur during that process.

The code calls the `bind()` function, which attempts to bind the socket to the specified address and port. The `bind()` function returns a value - if successful, it returns 0, and if it fails, it returns -1 (which is less than 0).

So the sequence is:

1. The code calls `bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))`
2. The operating system tries to perform the binding operation
3. The `bind()` function returns a value (0 for success, -1 for failure)
4. The code checks if this return value is less than 0 with `if (... < 0)`
5. If binding failed, it executes the error handling code inside the if block

If the binding is successful (returns 0), the condition is false, and the program continues to the next steps. The binding persists until the program closes the socket or terminates.

So you're right that the binding is established in this same line of code that's checking for errors, and if successful, that binding persists for the life of the socket. There's no need to establish it separately.

The key point is that the `< 0` check isn't checking if the connection is bad - it's checking if the attempt to bind the socket failed. If binding succeeds, the program continues with a properly bound socket ready for the next steps (listening and accepting connections).


### Listening
```c
listen(server_fd, 5)
```
- Start listening for connections
- `5` is the maximum number of pending connections

````c path=wiki/02-basic-socket-server.md mode=EXCERPT
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
````

Line-by-line explanation:

1. `if (listen(server_fd, 5) < 0) {`
   - The `listen()` function marks the socket as passive, ready to accept incoming connections
   - `server_fd` is our socket file descriptor created earlier
   - `5` is the backlog parameter - the maximum length of the queue of pending connections
   - This means up to 5 clients can be waiting to be accepted while you're processing another client
   - `< 0` checks if listen returned a negative value (error)
   - The entire line means: "If setting the socket to listening mode fails..."

2. `perror("Listen failed");`
   - Prints an error message to stderr with the specific system error that occurred
   - For example: "Listen failed: Too many open files"
   - Helps diagnose what went wrong with the listen call

3. `close(server_fd);`
   - Releases the socket resource by closing the file descriptor
   - This is proper cleanup to avoid resource leaks
   - Always important to close sockets when you're done with them or when errors occur

4. `exit(1);`
   - Terminates the program with exit code 1 (indicating error)
   - Prevents the program from continuing after a critical failure
   - The non-zero exit code can be detected by scripts or parent processes

This code:
1. Attempts to put the socket in listening mode with a queue of up to 5 pending connections
2. Checks if that operation failed
3. If it failed, prints an error message, cleans up resources, and exits the program
4. If successful, the program continues to the next steps (accepting connections)

The listen call is essential because it transforms the socket from an active socket (that could connect to other servers) to a passive socket that can accept incoming connections.


### Accepting Connections
```c
client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
```
- Waits for a client to connect
- Returns a new socket for communicating with that client

The main server loop handles client connections one at a time:

```c
while (1) {
    printf("Waiting for connection...\n");
    
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Accept failed");
        continue;
    }
```

This begins an infinite loop that waits for client connections. The `accept()` function blocks until a client connects, then returns a new socket file descriptor (`client_fd`) for that specific connection. If accept fails, it prints an error and continues waiting.

let me clarify what `&client_len` is for:

The `&client_len` parameter is not where the fail condition is defined. It serves a different purpose:

```c
client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
```

`&client_len` is a pointer to a variable that:

1. **Before the call**: Tells the `accept()` function how much space is available in the `client_addr` structure (the size of the buffer where the client's address will be stored)

2. **After the call**: Gets updated by `accept()` to indicate how many bytes were actually written into the `client_addr` structure

This is necessary because:
- Different address families (IPv4, IPv6, etc.) have different sized address structures
- The `accept()` function needs to know how much space it has to work with
- The function also needs to tell you how much of that space it actually used

The fail condition is determined by the return value of `accept()` itself (stored in `client_fd`), not by `client_len`:
- If `accept()` succeeds, it returns a new socket file descriptor (a non-negative integer)
- If `accept()` fails, it returns -1

That's why the code checks `if (client_fd < 0)` to determine if the accept operation failed.

So at a high level, this code:
1. Waits for and accepts a connection on the listening socket
2. Stores the connecting client's address information in `client_addr`
3. Uses `client_len` to manage the size of the address structure
4. Creates a new socket (`client_fd`) for communicating with this specific client
5. Checks if the accept operation failed by examining the return value

The `&client_len` is just a technical requirement of the `accept()` function to properly handle address information.

Let me clarify what `&client_len` is actually for:

`&client_len` has nothing to do with the data being sent or received, or with packet sizes. It's specifically about the address information of the connecting client.

Here's what it actually does:

1. `client_addr` is a structure that will store the client's address information (IP address, port number, etc.)

2. `client_len` initially contains the size of the `client_addr` structure (how much memory is allocated for it)

3. When you pass `&client_len` to `accept()`:
   - You're telling `accept()` how big your `client_addr` buffer is
   - `accept()` will update the value to tell you how many bytes it actually wrote into that buffer

This is necessary because:
- The socket API supports different types of addresses (IPv4, IPv6, Unix domain sockets)
- These different address types have different sized structures
- The `accept()` function needs to know how much space you've provided
- It also needs to tell you how much of that space it actually used

It has nothing to do with:
- The size of data being transferred between client and server
- Packet sizes or splitting data into multiple packets
- Buffer space for actual communication

The actual data transfer between client and server happens later using the `read()` and `write()` functions on the `client_fd` socket, and that's where buffer sizes for data would come into play.

So `&client_len` is purely about managing the size of the address structure during the connection establishment phase, not about data transfer capacity.

The `&client_len` is specifically about the metadata of the connection - it deals with the address information (the "who" and "where" of the client), not the actual data being transferred.

More specifically:

1. It helps manage the storage of connection metadata like:
   - The client's IP address
   - The client's port number
   - The address family (IPv4, IPv6, etc.)

2. It's a two-way communication with the `accept()` function:
   - You tell `accept()` how much space you've allocated for this metadata
   - `accept()` tells you how much of that space it actually used

This is purely about properly handling the addressing information that identifies who connected to your server, which is separate from the actual message data that will be exchanged later through the established connection.

Think of it like an envelope for a letter:
- The address on the envelope (managed by `&client_len`) tells you who sent it
- The actual letter inside (handled by `read()`/`write()`) is the data being communicated

The `&client_len` parameter helps manage the "envelope" part, not the "letter" part.

````
EXAMPLE: IPv4 CONNECTION ACCEPT PROCESS
=======================================

Before accept() call:
-------------------

Server side:                                 
                                             
socklen_t client_len = sizeof(struct sockaddr_in);  // Initially set to 16 bytes (size of IPv4 structure)
struct sockaddr_in client_addr;                     // Empty structure to be filled with client info
                                             
                                             
During accept() call:                        
--------------------                         
                                             
CLIENT (IP: 192.168.1.5:49152)               SERVER (IP: 192.168.1.10:8080)
+------------------------+                   +------------------------+
|                        |                   |                        |
|  Initiates connection  |------------------>|  accept() function     |
|                        |                   |  receives connection   |
+------------------------+                   +------------------------+
                                             |                        |
                                             |  Fills client_addr     |
                                             |  with:                 |
                                             |  - Family: AF_INET     |
                                             |  - IP: 192.168.1.5     |
                                             |  - Port: 49152         |
                                             |                        |
                                             |  Updates client_len    |
                                             |  to actual bytes used  |
                                             |  (still 16 for IPv4)   |
                                             +------------------------+


After accept() call:
------------------

client_fd = 4                // New socket descriptor for this specific client
client_len = 16              // Size of the address data that was written
client_addr contains:        // Structure now filled with client's address info
  sin_family = AF_INET       // Address family (IPv4)
  sin_port = 49152           // Client's port number (in network byte order)
  sin_addr = 192.168.1.5     // Client's IP address (in network byte order)


IMPORTANT NOTES:
--------------

1. client_len is NOT about the data being transferred
2. It's only about the address information (who is connecting)
3. For IPv4, the size is always 16 bytes
4. For IPv6, it would be 28 bytes (different structure size)
5. The actual data exchange happens later with read()/write()
````

To further clarify:

1. The `client_len` variable is only involved during the connection establishment phase.

2. It has nothing to do with the actual messages or data that will be sent between client and server after the connection is established.

3. For IPv4 connections, the address information is always the same size (16 bytes), but the API was designed to handle different address families with different sized structures.

4. After `accept()` completes, `client_len` has served its purpose - it's not used for the actual data communication.

The actual data exchange happens with completely different mechanisms:

```c
// Reading data FROM client
int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);


These operations use the `client_fd` socket and separate buffers for the actual message data, completely independent from the address information that was handled by `client_addr` and `client_len`.


```c
    printf("Client connected from %s\n", inet_ntoa(client_addr.sin_addr));
    
    // Read data from client
    int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);
```

Once a client connects, the server prints the client's IP address. It then reads data from the client into a buffer. If data was successfully read, it adds a null terminator to make it a valid C string and prints the received message.

**Difference between buffer and BUFFER_SIZE**

The difference between `buffer` and `BUFFER_SIZE` is that:

- `buffer` is the actual array (the storage space itself) where data will be temporarily held
- `BUFFER_SIZE` is just a constant that defines how large that storage space is

Let's break it down with the relevant code:

```c
#define BUFFER_SIZE 1024        // This defines a constant with value 1024
char buffer[BUFFER_SIZE];       // This creates an array named "buffer" with 1024 bytes of space
```

Think of it this way:
- `BUFFER_SIZE` is like saying "I want a bucket that can hold 1024 gallons"
- `buffer` is the actual bucket itself

When you use the buffer:
```c
int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
```

You're telling the `read()` function:
1. "Put the data from the client into this bucket (`buffer`)"
2. "But don't put more than 1023 gallons (`BUFFER_SIZE - 1`) in it"

The reason for using `BUFFER_SIZE - 1` instead of just `BUFFER_SIZE` is to leave room for a null terminator (`\0`) at the end, which is needed to properly terminate C strings.

After reading data:
```c
buffer[bytes_read] = '\0';  // Add null terminator at the end of the actual data
```

This places the null terminator right after the last byte that was read, making the data in the buffer a proper C string that can be safely used with string functions.

So in summary:
- `BUFFER_SIZE` is just a number (a constant)
- `buffer` is the actual memory space (an array of characters)


// Writing data TO client
write(client_fd, response, strlen(response));

```c
        // Send response
        const char* response = "Hello from server!\n";
        write(client_fd, response, strlen(response));
    }
```

The server sends a simple response message back to the client using the `write()` function.

**Why is response defined as const char*?**

Let's break down why it's defined as `const char*` instead of just `char response`:

```c
const char* response = "Hello from server!\n";
```

There are several important reasons for this:

1. **String vs. Character**:
   - `char response` would define a single character variable, not a string
   - To store "Hello from server!\n", we need multiple characters (a string)

2. **String Literals in C**:
   - When you write `"Hello from server!\n"`, C creates this as a string literal in memory
   - String literals in C are stored as arrays of characters terminated by a null character (`\0`)
   - These literals are stored in a special read-only section of memory

3. **Pointers to String Literals**:
   - `const char*` means "a pointer to character data that shouldn't be modified"
   - This pointer stores the memory address where the string literal begins
   - It doesn't copy the string; it just points to where it's stored

4. **The `const` Keyword**:
   - The `const` indicates that the characters pointed to shouldn't be modified
   - This is important because string literals are typically stored in read-only memory
   - Attempting to modify a string literal can cause a program crash

If we wanted to use a modifiable character array instead, we would need:

```c
char response[] = "Hello from server!\n";
```

This creates a copy of the string in a modifiable array. However, for a simple, fixed response that won't change, using `const char*` is more memory-efficient because:
- It doesn't create a copy of the string
- It just points to the existing string literal in memory

In this server code, since the response is fixed and doesn't need to be modified, using `const char*` is the appropriate choice.

```c
write(client_fd, response, strlen(response));
```

1. It sends the string literal "Hello from server!\n" to the client
2. It includes the length of the string (calculated by `strlen(response)`) so the system knows exactly how many bytes to transmit

The `write()` function takes three parameters:
- `client_fd`: The socket connected to the client (where to send the data)
- `response`: A pointer to the data to send (the string "Hello from server!\n")
- `strlen(response)`: The number of bytes to send (the length of the string)

This is a straightforward operation that sends the response message back to the client that connected to the server. The client will receive exactly those bytes containing the text message.

The client doesn't actually receive the length as separate information - the length is just used by the `write()` function to know how much data to transfer. The client's `read()` function will return the number of bytes it received.




```c
    // Close client connection
    close(client_fd);
    printf("Client disconnected\n");
}

close(server_fd);
return 0;
```

After handling the client's request, the server closes the client socket and prints a disconnection message. The loop then starts over, waiting for the next client. The final two lines would only execute if the infinite loop were somehow terminated.

The `close(server_fd)` and `return 0` lines will never be executed in this code as written.

Since the while loop condition is `while(1)`, which is always true, the loop will run indefinitely and the program will never reach the code after the loop unless:

1. The program is terminated externally (e.g., by pressing Ctrl+C)
2. The program calls `exit()` somewhere inside the loop
3. A system call inside the loop fails catastrophically

These lines are likely included for:

1. **Completeness**: To show proper resource cleanup if the loop were to exit
2. **Future-proofing**: In case the loop condition is changed later
3. **Educational purposes**: To demonstrate proper socket cleanup

In a more sophisticated server, you might have a condition to exit the loop, such as:
- A shutdown signal from the operating system
- A specific command received from a client
- A maximum number of connections reached

In those cases, these cleanup lines would be important. As the code stands now, though, you're correct that they will never be executed.


## Step 3: Compile and Test

### Create a Makefile
Create `Makefile`:
```makefile
CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRCDIR=src
SOURCES=$(SRCDIR)/server.c
TARGET=server

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
```
**Makefile Line-by-Line Explanation**

```makefile
CC=gcc
```
- Defines the C compiler to use (GNU C Compiler)
- This variable can be referenced later as `$(CC)`

```makefile
CFLAGS=-Wall -Wextra -std=c99
```
- Defines compiler flags:
  - `-Wall`: Enable all common warning messages
  - `-Wextra`: Enable extra warning messages beyond `-Wall`
  - `-std=c99`: Use the C99 standard (a version of the C language)
- These flags help catch potential bugs and ensure code quality

```makefile
SRCDIR=src
```
- Defines the directory where source files are located
- Makes the Makefile more maintainable by centralizing this path

```makefile
SOURCES=$(SRCDIR)/server.c
```
- Lists all source files to compile
- Uses the `SRCDIR` variable to build the full path
- Currently only includes one file: `src/server.c`

```makefile
TARGET=server
```
- Names the final executable that will be created
- The program will be named `server`

```makefile
$(TARGET): $(SOURCES)
```
- This is a rule declaration stating:
  - `$(TARGET)` (server) is the target to build
  - `$(SOURCES)` (src/server.c) are the prerequisites
  - If any source file changes, this rule will execute

```makefile
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)
```
- The command to execute when building the target
- Note: This line MUST start with a tab character, not spaces
- Expands to: `gcc -Wall -Wextra -std=c99 -o server src/server.c`
- This compiles the source files and creates the executable

```makefile
clean:
```
- Defines a target named `clean` with no prerequisites
- This is a utility target to remove generated files

```makefile
	rm -f $(TARGET)
```
- The command for the `clean` target
- Removes the executable file (`server`)
- `-f` flag makes rm not complain if the file doesn't exist

```makefile
.PHONY: clean
```
- Declares `clean` as a "phony" target
- This means `clean` doesn't represent a file to be created
- Prevents issues if a file named "clean" ever exists in the directory
- Ensures the `clean` command always runs when requested

This Makefile provides two main functions:
1. Building the server executable (`make` or `make server`)
2. Cleaning up generated files (`make clean`)

It's a simple but effective build system for this small project.


### Compile
```bash
make
```

### Run the Server
```bash
./server
```

You should see:
```
Starting server on port 8080...
Server listening on port 8080
Waiting for connection...
```

## Step 4: Test Your Server

### Option 1: Using telnet
In another terminal:
```bash
telnet localhost 8080
```

Type something and press Enter. You should see your message echoed back!

### Option 2: Using netcat
```bash
echo "Hello server!" | nc localhost 8080
```

### Option 3: Using curl
```bash
curl http://localhost:8080
```

## Common Issues and Solutions

### "Address already in use" Error
If you see this error, either:
1. Wait a minute and try again
2. Add this code after creating the socket:
```c
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

### "Permission denied" Error
- Try a different port (like 8081) if 8080 is restricted
- Or run with sudo (not recommended for development)

### Server Doesn't Respond
- Check if the server is running
- Make sure you're connecting to the right port
- Check firewall settings

## What's Happening Behind the Scenes

1. **Server starts**: Creates a socket and binds to port 8080
2. **Client connects**: Browser/curl creates a connection to your server
3. **Data exchange**: Client sends data, server reads it and responds
4. **Connection closes**: Server closes the connection after responding

## Limitations of This Server

- Handles only one client at a time
- Doesn't understand HTTP protocol
- Sends the same response regardless of request
- No error handling for malformed data

## Next Steps

Great! You now have a working TCP server. In the next phase, we'll make it understand HTTP requests.

## Exercises

1. **Modify the response**: Change the server to send back the current time
2. **Echo server**: Make the server send back exactly what the client sent
3. **Connection counter**: Keep track of how many clients have connected

### Exercise Solutions

**Echo Server Modification:**
```c
// Replace the response section with:
write(client_fd, buffer, bytes_read);
```

**Connection Counter:**
```c
// Add at the top of main():
int connection_count = 0;

// In the accept loop:
connection_count++;
printf("Connection #%d from %s\n", connection_count, inet_ntoa(client_addr.sin_addr));
```

## Troubleshooting Checklist

- [ ] Code compiles without errors
- [ ] Server starts and shows "listening" message
- [ ] Can connect with telnet/curl
- [ ] Server responds to requests
- [ ] Can stop server with Ctrl+C

---

**Previous:** [← Getting Started](01-getting-started.md) | **Next:** [Phase 2: HTTP Request Parsing →](03-http-parsing.md)
