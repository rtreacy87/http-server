
```bash
make
gcc -Wall -Wextra -std=c99 -Iinclude -o server src/server.c src/http_parser.c src/parse_http_request_supplement.c src/send_http_response_supplement.c src/router.c src/file_server.c
src/file_server.c: In function ‘get_mime_type’:
src/file_server.c:43:13: warning: implicit declaration of function ‘strcasecmp’; did you mean ‘strncmp’? [-Wimplicit-function-declaration]
   43 |         if (strcasecmp(extension, mime_types[i].extension) == 0) {
      |             ^~~~~~~~~~
      |             strncmp
src/file_server.c: In function ‘serve_static_file_handler’:
src/file_server.c:93:26: warning: implicit declaration of function ‘strdup’; did you mean ‘strcmp’? [-Wimplicit-function-declaration]
   93 |         response->body = strdup("File not found");
      |                          ^~~~~~
      |                          strcmp
src/file_server.c:93:24: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   93 |         response->body = strdup("File not found");
      |                        ^
src/file_server.c:107:24: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
  107 |         response->body = strdup("File not found");
      |                        ^
src/file_server.c:122:24: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
  122 |         response->body = strdup("Internal server error");
      |                        ^
src/file_server.c: In function ‘serve_static_file’:
src/file_server.c:148:9: warning: unused variable ‘original_status’ [-Wunused-variable]
  148 |     int original_status = response->status_code;
      |         ^~~~~~~~~~~~~~~
/usr/bin/ld: /tmp/ccNgfxAi.o: in function `build_file_path':
file_server.c:(.text+0x139): undefined reference to `get_default_file_path'
/usr/bin/ld: file_server.c:(.text+0x153): undefined reference to `construct_full_path'
/usr/bin/ld: /tmp/ccNgfxAi.o: in function `serve_static_file_handler':
file_server.c:(.text+0x3c1): undefined reference to `validate_file'
/usr/bin/ld: file_server.c:(.text+0x4ad): undefined reference to `read_file_content'
collect2: error: ld returned 1 exit status
```


```bash
 make
gcc -Wall -Wextra -std=c99 -Iinclude -o server src/server.c src/http_parser.c src/parse_http_request_supplement.c src/send_http_response_supplement.c src/router.c src/file_server.c src/serve_static_file_supplement.c src/build_file_path_supplement.c
src/file_server.c: In function ‘get_mime_type’:
src/file_server.c:43:13: warning: implicit declaration of function ‘strcasecmp’; did you mean ‘strncmp’? [-Wimplicit-function-declaration]
   43 |         if (strcasecmp(extension, mime_types[i].extension) == 0) {
      |             ^~~~~~~~~~
      |             strncmp
src/file_server.c: In function ‘serve_static_file_handler’:
src/file_server.c:93:26: warning: implicit declaration of function ‘strdup’; did you mean ‘strcmp’? [-Wimplicit-function-declaration]
   93 |         response->body = strdup("File not found");
      |                          ^~~~~~
      |                          strcmp
src/file_server.c:93:24: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
   93 |         response->body = strdup("File not found");
      |                        ^
src/file_server.c:107:24: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
  107 |         response->body = strdup("File not found");
      |                        ^
src/file_server.c:122:24: warning: assignment to ‘char *’ from ‘int’ makes pointer from integer without a cast [-Wint-conversion]
  122 |         response->body = strdup("Internal server error");
      |                        ^
src/file_server.c: In function ‘serve_static_file’:
src/file_server.c:148:9: warning: unused variable ‘original_status’ [-Wunused-variable]
  148 |     int original_status = response->status_code;
      |         ^~~~~~~~~~~~~~~
```

