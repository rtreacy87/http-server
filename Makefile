CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRCDIR=src
SOURCES=$(SRCDIR)/server.c
TARGET=server

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean

