# HTTP Server Wiki Series

## Welcome to Building an HTTP Server in C!

This comprehensive wiki series will guide you through building a complete HTTP server from scratch in C. By the end, you'll have a working web server that can serve static files, handle dynamic content, and understand the fundamentals of web communication.

## 📚 Wiki Series Overview

### [1. Getting Started](01-getting-started.md)
**Start here!** Learn what you'll build, set up your development environment, and understand HTTP basics.

**What you'll learn:**
- Project overview and goals
- Development environment setup
- HTTP protocol fundamentals
- Project structure

### [2. Phase 1: Basic Socket Server](02-basic-socket-server.md)
Build the networking foundation using Berkeley sockets.

**What you'll learn:**
- Socket programming basics
- TCP server creation
- Client connection handling
- Basic network I/O

### [3. Phase 2: HTTP Request Parsing](03-http-parsing.md)
Transform your socket server into an HTTP server by parsing requests and generating responses.

**What you'll learn:**
- HTTP request structure
- Request parsing implementation
- HTTP response generation
- Basic routing

### [4. Phase 3: Static File Serving](04-static-files.md)
Add the ability to serve HTML, CSS, JavaScript, and other static files from disk.

**What you'll learn:**
- File I/O operations
- MIME type detection
- Security considerations
- Directory traversal protection

### [5. Phase 4: Dynamic Content Generation](05-dynamic-content.md)
Create a flexible routing system for APIs and dynamic web pages.

**What you'll learn:**
- Advanced routing systems
- API endpoint creation
- Form data handling
- Query parameter parsing

### [6. Phase 5: Advanced Features](06-advanced-features.md)
Add production-ready features like persistent connections, authentication, and security headers.

**What you'll learn:**
- HTTP/1.1 Keep-Alive connections
- Basic authentication
- Security headers
- Connection management

### [7. Testing and Debugging Guide](07-testing-guide.md)
Learn how to test your server thoroughly and debug common issues.

**What you'll learn:**
- Unit testing strategies
- Integration testing
- Load testing
- Debugging techniques

### [8. Security Best Practices](08-security-guide.md)
Understand web security vulnerabilities and how to protect against them.

**What you'll learn:**
- Common web vulnerabilities
- Input validation
- Security headers
- Authentication security

### [9. Troubleshooting Guide](09-troubleshooting.md)
Diagnose and fix common problems you might encounter.

**What you'll learn:**
- Compilation issues
- Runtime problems
- Network debugging
- Performance optimization

### [10. Project Summary](10-project-summary.md)
Reflect on what you've built and explore next steps.

**What you'll learn:**
- Architecture overview
- Skills gained
- Real-world applications
- Future enhancements

## 🎯 Learning Path

### For Complete Beginners
1. Start with [Getting Started](01-getting-started.md)
2. Follow each phase in order (1-5)
3. Use the [Troubleshooting Guide](09-troubleshooting.md) when needed
4. Read [Security Guide](08-security-guide.md) for awareness

### For Experienced Programmers
1. Skim [Getting Started](01-getting-started.md) for project overview
2. Jump to phases that interest you
3. Focus on [Advanced Features](06-advanced-features.md) and [Security](08-security-guide.md)
4. Use [Testing Guide](07-testing-guide.md) for quality assurance

### For Security-Focused Learning
1. Read [Getting Started](01-getting-started.md) for context
2. Build basic functionality (Phases 1-3)
3. Deep dive into [Security Guide](08-security-guide.md)
4. Implement security features from [Advanced Features](06-advanced-features.md)

## 🛠️ Prerequisites

### Required Knowledge
- Basic C programming (variables, functions, structs, pointers)
- Command line usage
- Text editor familiarity

### Don't Worry If You Don't Know
- Advanced networking concepts
- HTTP protocol details
- Socket programming
- Web security

**We'll teach you everything you need to know!**

## 🚀 Quick Start

1. **Set up your environment** (see [Getting Started](01-getting-started.md))
2. **Create project directory:**
   ```bash
   mkdir http-server
   cd http-server
   mkdir src include tests static
   ```
3. **Start with Phase 1:** [Basic Socket Server](02-basic-socket-server.md)

## 📋 What You'll Build

By the end of this series, your HTTP server will have:

### Core Features
- ✅ HTTP/1.1 protocol support
- ✅ GET and POST request handling
- ✅ Static file serving (HTML, CSS, JS, images)
- ✅ Dynamic content generation
- ✅ API endpoints with JSON responses
- ✅ Form data processing

### Advanced Features
- ✅ Persistent connections (Keep-Alive)
- ✅ Basic authentication
- ✅ Security headers
- ✅ Request logging
- ✅ Error handling
- ✅ Connection timeouts

### Security Features
- ✅ Directory traversal protection
- ✅ Input validation
- ✅ Rate limiting basics
- ✅ Security headers (XSS, CSRF protection)

## 🎓 Skills You'll Gain

### Technical Skills
- **Network Programming**: Socket APIs, TCP/IP
- **HTTP Protocol**: Request/response cycle, headers, status codes
- **C Programming**: Memory management, string handling, modular design
- **Web Security**: Common vulnerabilities and protections
- **System Programming**: File I/O, process management, error handling

### Conceptual Understanding
- How web servers work internally
- Client-server architecture
- Protocol design principles
- Security considerations
- Performance optimization

## 🔧 Tools You'll Use

- **C Compiler**: GCC or Clang
- **Text Editor**: Any editor (VS Code, vim, nano)
- **Testing Tools**: curl, browser, custom test scripts
- **Debugging Tools**: GDB, Valgrind
- **Build System**: Make

## 📖 How to Use This Wiki

### Reading Tips
- Each wiki builds on previous ones
- Code examples are complete and tested
- Common issues are addressed in each section
- Links between wikis help you navigate

### Hands-On Approach
- Type out code examples (don't just copy-paste)
- Test each phase before moving to the next
- Experiment with modifications
- Use the troubleshooting guide when stuck

### Getting Help
- Check the [Troubleshooting Guide](09-troubleshooting.md) first
- Review error messages carefully
- Test with simple cases before complex ones
- Don't hesitate to start over if needed

## 🌟 Success Tips

1. **Take Your Time**: Understanding is more important than speed
2. **Test Frequently**: Verify each feature works before adding more
3. **Read Error Messages**: They usually tell you exactly what's wrong
4. **Start Simple**: Get basic functionality working first
5. **Ask Questions**: Use the troubleshooting guide and online resources

## 🔗 Additional Resources

### Documentation
- [C Reference](https://en.cppreference.com/w/c)
- [HTTP Specification](https://tools.ietf.org/html/rfc7230)
- [Socket Programming Guide](https://beej.us/guide/bgnet/)

### Tools
- [curl](https://curl.se/) - Command-line HTTP client
- [Postman](https://www.postman.com/) - GUI HTTP client
- [Wireshark](https://www.wireshark.org/) - Network protocol analyzer

## 🤝 Contributing

Found an error or want to improve the wiki?
- Check for typos and technical accuracy
- Suggest clearer explanations
- Add more examples or test cases
- Share your experience and tips

## 📄 License

This educational content is provided for learning purposes. Feel free to use, modify, and share with others who want to learn systems programming.

---

## Ready to Start?

🚀 **Begin your journey:** [Getting Started →](01-getting-started.md)

**Happy coding!** Building an HTTP server from scratch is challenging but incredibly rewarding. You'll gain deep insights into how the web works and develop valuable systems programming skills.

---

*Last updated: 2024*
