#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../include/http.h"
#include "../include/build_file_path_supplement.h"


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


