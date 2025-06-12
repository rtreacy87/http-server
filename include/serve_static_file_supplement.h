#ifndef SERVE_STATIC_FILE_SUPPLEMENT_H
#define SERVE_STATIC_FILE_SUPPLEMENT_H

#include "../include/http.h"


int validate_file(const char* file_path, struct stat* file_stat);
void* read_file_content(const char* file_path, size_t file_size, size_t* bytes_read);

#endif
