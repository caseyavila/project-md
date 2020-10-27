CC ?= gcc
CFLAGS += `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0`
LDLIBS += `pkg-config --libs gtk+-3.0 webkit2gtk-4.0` -lm

SRC = $(wildcard src/*.c) \
      $(wildcard src/scidown/src/*.c) \
      $(wildcard src/scidown/src/charter/src/[^main]*.c) \
      $(wildcard src/scidown/src/charter/src/csv_parser/csvparser.c) \
      $(wildcard src/scidown/src/charter/src/tinyexpr/tinyexpr.c)

OBJ = $(SRC:.c=.o)

project-md: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJ) project-md

.PHONY: clean
