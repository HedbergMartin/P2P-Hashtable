CC=gcc

CFLAGS= -std=gnu11 -pedantic -Wall #-Wextra -Werror -Wmissing-declarations -Wmissing-prototypes -Werror-implicit-function-declaration -Wreturn-type -Wparentheses -Wunused -Wold-style-definition -Wundef -Wshadow -Wstrict-prototypes -Wswitch-default -Wunreachable-code

NODE_SRC=node/node.c
PDU_PARSER_SRC=pdu/pdu_parser.c

all: node

node: node.o pdu_parser.o
	$(CC) $(CFLAGS) -o $@ $^

node.o: $(NODE_SRC) $(node/node.h pdu/headers/*.h)
	$(CC) $(CFLAGS) -c $<

pdu_parser.o: $(PDU_PARSER_SRC) $(pdu/headers/*.h)
	$(CC) $(CFLAGS) -c $<

valgrind:
	valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./webserver 4021 serverstuff

clean:
	rm -f node *.o core