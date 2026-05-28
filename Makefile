CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Wformat=2 \
           -Wno-unused-parameter -Wshadow -Wwrite-strings \
           -Wstrict-prototypes -Wold-style-definition \
           -Wredundant-decls -Wnested-externs -Wmissing-include-dirs
CFLAGS += -O2 -Iinclude -MMD -MP
LDFLAGS += $(shell pkg-config --libs x11 xft freetype2 fontconfig)
CFLAGS += $(shell pkg-config --cflags x11 xft freetype2 fontconfig)

PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
DEP = $(OBJ:.o=.d)
BIN = oxbar

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c -o $@ $<

-include $(DEP)

install: all
	install -Dm755 $(BIN) $(DESTDIR)$(BINDIR)/$(BIN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)

clean:
	rm -rf build $(BIN)

.PHONY: all install uninstall clean
