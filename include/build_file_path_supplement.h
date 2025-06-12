#ifndef BUILD_FILE_PATH_SUPPLEMENT_H
#define BUILD_FILE_PATH_SUPPLEMENT_H

const char* get_default_file_path(const char* uri);
char* construct_full_path(const char* base_dir, const char* file_path);

#endif
