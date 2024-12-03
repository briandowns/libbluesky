cc = cc

NAME = libbluesky

UNAME_S = $(shell uname -s)

CFLAGS  = -std=c17 -O3 -fPIC -Wall -Wextra
ifeq ($(UNAME_S),FreeBSD)
	CFLAGS += $(shell pkg-config --cflags --libs libcurl)
endif
LDFLAGS = -lcurl -ljansson

# respect traditional UNIX paths
INCDIR  = /usr/local/include
LIBDIR  = /usr/local/lib

ifeq ($(UNAME_S),Darwin)
$(NAME).dylib: clean
	$(CC) -dynamiclib -o $@ bluesky.c $(CFLAGS) $(LDFLAGS)
endif
ifeq ($(UNAME_S),Linux)
$(NAME).so: clean
	$(CC) -shared -o $@ bluesky.c $(CFLAGS) $(LDFLAGS)
endif

.PHONY: tests
tests: clean
	$(CC) -o tests/tests tests/unity.c tests/bluesky_test.c bluesky.c $(CFLAGS) $(LDFLAGS)
	tests/tests
	rm -f tests/tests

.PHONY: valgrind
valgrind: tests
	valgrind --leak-check=full ./tests/tests 2>&1 | awk -F':' '/definitely lost:/ {print $2}'

.PHONY: install
install: 
	cp bluesky.h $(INCDIR)
ifeq ($(UNAME_S),Linux)
	cp bluesky.h $(INCDIR)
	cp $(NAME).so $(LIBDIR)
endif
ifeq ($(UNAME_S),Darwin)
	cp bluesky.h $(INCDIR)
	cp $(NAME).dylib $(LIBDIR)
endif

uninstall:
	rm -f $(INCDIR)/bluesky.h
ifeq ($(UNAME_S),Linux)
	rm -f $(INCDIR)/$(NAME).so
endif
ifeq ($(UNAME_S),Darwin)
	rm -f $(INCDIR)/$(NAME).dylib
endif

.PHONY: clean
clean:
	rm -f $(NAME).dylib
	rm -f $(NAME).so
	rm -f example
	rm -f tests/tests

.PHONY: example
example: clean
	$(CC) -o $@ bluesky.c example.c $(CFLAGS) $(LDFLAGS)