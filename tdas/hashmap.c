#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

typedef struct HashNode {
    char *key;
    void *value;
    struct HashNode *next;
} HashNode;

struct HashMap {
    HashNode **buckets;
    int capacity;
};

unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

HashMap *hashmap_create(int capacity) {
    HashMap *map = malloc(sizeof(HashMap));
    map->capacity = capacity;
    map->buckets = calloc(capacity, sizeof(HashNode *));
    return map;
}

void hashmap_insert(HashMap *map, char *key, void *value) {
    unsigned long index = hash_string(key) % map->capacity;

    HashNode *actual = map->buckets[index];

    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {
            actual->value = value;
            return;
        }
        actual = actual->next;
    }

    HashNode *nuevo = malloc(sizeof(HashNode));
    nuevo->key = key;
    nuevo->value = value;
    nuevo->next = map->buckets[index];

    map->buckets[index] = nuevo;
}

void *hashmap_search(HashMap *map, const char *key) {
    unsigned long index = hash_string(key) % map->capacity;

    HashNode *actual = map->buckets[index];

    while (actual != NULL) {
        if (strcmp(actual->key, key) == 0) {
            return actual->value;
        }
        actual = actual->next;
    }

    return NULL;
}

void hashmap_destroy(HashMap *map, int freeKeys) {
    if (map == NULL) return;

    for (int i = 0; i < map->capacity; i++) {
        HashNode *actual = map->buckets[i];

        while (actual != NULL) {
            HashNode *temp = actual;
            actual = actual->next;

            if (freeKeys) free(temp->key);
            free(temp);
        }
    }

    free(map->buckets);
    free(map);
}