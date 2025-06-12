CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRCDIR=src
INCDIR=include
SOURCES=$(SRCDIR)/server.c $(SRCDIR)/http_parser.c $(SRCDIR)/parse_http_request_supplement.c $(SRCDIR)/send_http_response_supplement.c $(SRCDIR)/router.c $(SRCDIR)/file_server.c $(SRCDIR)/serve_static_file_supplement.c $(SRCDIR)/build_file_path_supplement.c
TARGET=server

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -I$(INCDIR) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean

