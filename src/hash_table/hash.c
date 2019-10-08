/**
 * djb2 hash function. No idea what it does but it works!
 */
unsigned long hash_ssn(unsigned char *str) {

    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
