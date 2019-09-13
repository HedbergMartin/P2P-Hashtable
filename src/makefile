CC=gcc

CFLAGS= -std=gnu11 -pedantic -Wall -Wextra -Werror -Wmissing-declarations -Wmissing-prototypes -Werror-implicit-function-declaration -Wreturn-type -Wparentheses -Wunused -Wold-style-definition -Wundef -Wshadow -Wstrict-prototypes -Wswitch-default -Wunreachable-code

HEADERS = webserver.h
WEBSERVER_SRC=webserver.c

all: webserver

webserver: webserver.o
	$(CC) $(CFLAGS) -o $@ $^

webserver.o: $(WEBSERVER_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -c $<

valgrind:
	valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./webserver 4021 serverstuff