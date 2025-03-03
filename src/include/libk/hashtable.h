#pragma once

#include <stdint.h>

typedef struct {
    uint64_t key;
    uint64_t value;
} hashtable_entry_t;


typedef struct {
    hashtable_entry_t* entries;
    uint64_t size;
} hashtable_t;


uint64_t HASHTABLEGET(hashtable_t* table, uint64_t key);
void HASHTABLESET(hashtable_t* table, uint64_t key, uint64_t value);
void HASHTABLEDELETE(hashtable_t* table, uint64_t key);
hashtable_t* HASHTABLECREATE(uint64_t size);



