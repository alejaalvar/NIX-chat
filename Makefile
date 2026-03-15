CC = gcc

CFLAGS = -Wall -Wextra -Wshadow -Wunreachable-code -Wredundant-decls \
         -Wmissing-declarations -Wold-style-definition \
         -Wmissing-prototypes -Wdeclaration-after-statement \
         -Wno-return-local-addr -Wunsafe-loop-optimizations \
         -Wuninitialized -Werror

PROG1 = client
PROG2 = server

TAR_FILE = $(PROG1).tar.gz

all: $(PROG1) $(PROG2)

$(PROG1): $(PROG1).o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROG1).o: $(PROG1).c
	$(CC) $(CFLAGS) -c $<

$(PROG2): $(PROG2).o
	$(CC): $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROG2).o: $(PROG2).c
	$(CC): $(CFLAGS) -c $<

clean:
	rm -f $(PROG1) $(PROG2) *.o *~ \#*

tar: clean
	rm -f $(TAR_FILE)
	tar cvfa $(TAR_FILE) *.[ch] ?akefile
