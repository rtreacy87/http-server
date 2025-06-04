# Getting Started: Building Your First HTTP Server in C

## Welcome!

This wiki series will guide you through building a complete HTTP server in C from scratch. By the end, you'll have a working web server that can serve files, handle API requests, and understand the fundamentals of web communication.

## What You'll Learn

- How web servers work at a low level
- Socket programming in C
- HTTP protocol basics
- File I/O and memory management
- Security considerations for web servers

## Prerequisites

### Required Knowledge
- Basic C programming (variables, functions, structs, pointers)
- Basic command line usage
- Text editor familiarity

### Don't Worry If You Don't Know
- Advanced networking concepts (we'll explain everything)
- HTTP protocol details (we'll cover this step by step)
- Socket programming (we'll start from the basics)

## Setting Up Your Development Environment

### Required Tools
1. **C Compiler**: GCC or Clang
2. **Text Editor**: Any editor (VS Code, vim, nano, etc.)
3. **Terminal**: Command line access

### Installation Instructions

#### On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential
```

#### On macOS:
```bash
# Install Xcode command line tools
xcode-select --install
```

#### On Windows:
- Install WSL (Windows Subsystem for Linux) and follow Ubuntu instructions
- Or use MinGW-w64

### Verify Your Setup
```bash
gcc --version
```
You should see version information if GCC is installed correctly.

## Project Structure

Create your project directory:
```bash
mkdir http-server
cd http-server
```

We'll organize our code like this:
```
http-server/
├── src/           # Source code files
├── include/       # Header files
├── tests/         # Test files
├── static/        # Static files to serve
├── Makefile       # Build configuration
└── README.md      # Project documentation
```

Create the directories:
```bash
mkdir src include tests static
```

## Understanding HTTP Basics

Before we start coding, let's understand what HTTP is:

### What is HTTP?
HTTP (HyperText Transfer Protocol) is how web browsers and servers communicate. When you visit a website:

1. Your browser sends an HTTP **request** to the server
2. The server processes the request
3. The server sends back an HTTP **response**

### HTTP Request Example
```
GET /index.html HTTP/1.1
Host: example.com
User-Agent: Mozilla/5.0
Accept: text/html

```

This request has:
- **Method**: GET (asking for data)
- **Path**: /index.html (what file we want)
- **Version**: HTTP/1.1
- **Headers**: Additional information

### HTTP Response Example
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 43

<html><body>Hello World!</body></html>
```

This response has:
- **Status**: 200 OK (success)
- **Headers**: Information about the content
- **Body**: The actual content

## Your First Goal

In the next wiki, we'll create a simple server that:
1. Listens for connections on port 8080
2. Accepts incoming connections
3. Reads data from clients
4. Sends back a simple response

## Common Beginner Questions

**Q: Do I need to understand networking to follow this tutorial?**
A: No! We'll explain networking concepts as we go.

**Q: Is C too difficult for beginners?**
A: C can be challenging, but we'll take it step by step. The concepts you learn will help you understand how web servers work at a fundamental level.

**Q: Can I use this server in production?**
A: Our server is for learning. Production servers need many additional features for security and performance.

**Q: What if I get stuck?**
A: Each wiki includes troubleshooting sections and common mistakes to watch for.

## Next Steps

Ready to start coding? Head to [Phase 1: Basic Socket Server](02-basic-socket-server.md) to create your first network server!

## Quick Reference

### Useful Commands
```bash
# Compile a C program
gcc -o server server.c

# Run your server
./server

# Test with curl (in another terminal)
curl http://localhost:8080

# Stop your server
Ctrl+C
```

### Helpful Resources
- [C Reference](https://en.cppreference.com/w/c)
- [HTTP Specification](https://tools.ietf.org/html/rfc7230)
- [Socket Programming Tutorial](https://beej.us/guide/bgnet/)

---

**Next:** [Phase 1: Basic Socket Server →](02-basic-socket-server.md)
