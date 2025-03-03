#include <stdint.h>
#include <libk/hashtable.h>


uint64_t HASHTABLEGET(hashtable_t* table, uint64_t key) {
    for (uint64_t i = 0; i < table->size; i++) {
        if (table->entries[i].key == key) {
            return table->entries[i].value;
        }
    }
    return 0;
}

void HASHTABLESET(hashtable_t* table, uint64_t key, uint64_t value) {
    for (uint64_t i = 0; i < table->size; i++) {
        if (table->entries[i].key == key) {
            table->entries[i].value = value;
            return;
        }
    }
    table->entries[table->size].key = key;
    table->entries[table->size].value = value;
    table->size++;
}

void HASHTABLEDELETE(hashtable_t* table, uint64_t key) {
    for (uint64_t i = 0; i < table->size; i++) {
        if (table->entries[i].key == key) {
            for (uint64_t j = i; j < table->size - 1; j++) {
                table->entries[j] = table->entries[j + 1];
            }
            table->size--;
            return;
        }
    }
}



