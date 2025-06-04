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

### Socket Creation
```c
server_fd = socket(AF_INET, SOCK_STREAM, 0);
```
- `AF_INET`: Use IPv4
- `SOCK_STREAM`: Use TCP (reliable, ordered data)
- Returns a file descriptor (like a handle to the socket)

### Address Configuration
```c
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = INADDR_ANY;
server_addr.sin_port = htons(PORT);
```
- `sin_family`: IPv4
- `INADDR_ANY`: Listen on all network interfaces
- `htons(PORT)`: Convert port to network byte order

### Binding
```c
bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))
```
- Associates the socket with the address and port
- Like claiming a phone number

### Listening
```c
listen(server_fd, 5)
```
- Start listening for connections
- `5` is the maximum number of pending connections

### Accepting Connections
```c
client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
```
- Waits for a client to connect
- Returns a new socket for communicating with that client

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
