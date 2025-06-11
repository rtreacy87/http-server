#ifndef PARSE_HTTP_REQUEST_SUPPLEMENT_H
#define PARSE_HTTP_REQUEST_SUPPLEMENT_H

char* find_header_end(const char* raw_request);
char* find_request_line_end(const char* raw_request);
char* copy_line(const char* start, const char* end);
int parse_header(const char* header_line, http_request_t* request);

#endif
