#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct HashMap HashMap;

HashMap *hashmap_create(int capacity);

void hashmap_insert(HashMap *map, char *key, void *value);

void *hashmap_search(HashMap *map, const char *key);

void hashmap_destroy(HashMap *map, int freeKeys);

#endif