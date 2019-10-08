#ifndef __HASH_TABLE__
#define __HASH_TABLE__

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

/**
 * Written by klasa and kuba for the course 5DV197. Use and modifications
 * are free within the scope of the course.
 *
 * klasa@cs.umu.se
 * kuba@cs.umu.se
 */

typedef uint8_t hash_t;

struct table_entry {
    char* ssn;
    char* name;
    char* email;
    struct table_entry* next;
};

typedef hash_t hash_function(char*);

struct hash_table {
    struct table_entry** entries;
    uint32_t num_entries;
    uint32_t max_entries;
    hash_function* hash_func;
};

/**
 * Creates a new hash table. The returned hash_table is allocated and must be freed with
 * table_free
 *
 * @param hash_func The hash function to use for entries
 * @param max_size The amount of buckets in the hash table
 */
struct hash_table* table_create(hash_function* hash_func, uint32_t max_size);

/**
 * Inserts a new entry into the hash table. The SSN is hashed and used as a key.
 */
void table_insert(struct hash_table* table, char* ssn, char* name, char* email);

/**
 * Removes the entry with the given SSN, if it exists in the table.
 */
void table_remove(struct hash_table* table, char* ssn);

/**
 * Gets a table entry from a given SSN. Returns null if non-existent.
 */
struct table_entry* table_lookup(struct hash_table* table, char* ssn);

/**
 * Frees all the resources used by the hash table.
 */
void table_free(struct hash_table* table);

/**
 * Iterates the elements of the table.
 * Note, not thread safe.
 */
struct table_entry* get_entry_iterator(struct hash_table* table);

/**
 * Frees a table entry.
 */
void entry_free(struct table_entry* e);

#endif