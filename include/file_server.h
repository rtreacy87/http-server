#ifndef FILE_SERVER_H
#define FILE_SERVER_H

#include "http.h"

void serve_static_file_handler(http_request_t* request, http_response_t* response);

#endif
