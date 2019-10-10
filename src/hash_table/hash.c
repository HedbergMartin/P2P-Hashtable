#include "../headers/hash.h"

/**
 * djb2 hash function. No idea what it does but it works!
 */
hash_t hash_ssn(unsigned char *str) {

    hash_t hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
