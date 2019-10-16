CC=gcc

CFLAGS= -std=gnu11 -pedantic -Wall -Wextra -Werror -Wmissing-declarations -Wmissing-prototypes -Werror-implicit-function-declaration -Wreturn-type -Wparentheses -Wunused -Wold-style-definition -Wundef -Wshadow -Wstrict-prototypes -Wswitch-default -Wunreachable-code

NODE_SRC=node.c
PDU_HANDLER_SRC=pdu_handler.c
PDU_PARSER_SRC=pdu_parser.c
PDU_SENDER_SRC=pdu_sender.c
SIGHANT_SRC=sighant.c
HASH_TABLE_SRC=hash_table/hash_table.c
HASH_SRC=hash_table/hash.c
HEADERS=headers/*.h

all: node

node: node.o pdu_handler.o pdu_sender.o pdu_parser.o hash_table.o hash.o
	$(CC) $(CFLAGS) -o $@ $^

node.o: $(NODE_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -c $<

pdu_handler.o: $(PDU_HANDLER_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -c $<

pdu_sender.o: $(PDU_SENDER_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -c $<

pdu_parser.o: $(PDU_PARSER_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -c $<

hash_table.o: $(HASH_TABLE_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -c $<

hash.o: $(HASH_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -c $<

valgrind:
	valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./webserver 4021 serverstuff

clean:
	rm -f node *.o core