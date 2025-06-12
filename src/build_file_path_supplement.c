#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
}
