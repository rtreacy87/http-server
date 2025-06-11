#ifndef SEND_HTTP_RESPONSE_SUPPLEMENT_H
#define SEND_HTTP_RESPONSE_SUPPLEMENT_H

void update_status_line(int client_fd, http_response_t* response);
void add_headers(int client_fd, http_response_t* response);
void update_headers(int client_fd, http_response_t* response);
void update_content_length(int client_fd, http_response_t* response);
void write_body(int client_fd, http_response_t* response);

#endif
