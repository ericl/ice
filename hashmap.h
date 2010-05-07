#ifndef HASHMAP_H
#define HASHMAP_H

#include <state.h>
#include <common.h>
#include <omp.h>
#include <string.h>

typedef struct entry {
	coord_t *bits;
	struct entry *next;
} entry_t;

typedef struct hashmap {
	entry_t **entries;
	int capacity;
	int size;
	int maxlen;
#if PARALLEL
	omp_lock_t lock;
#endif
} hashmap_t;

hashmap_t *create_hashmap() {
	hashmap_t *map = malloc(sizeof(hashmap_t));
	map->size = 0;
	map->maxlen = 0;
	map->capacity = HASHMAP_DEFAULT_CAPACITY;
	map->entries = calloc(HASHMAP_DEFAULT_CAPACITY, sizeof(entry_t));
#if PARALLEL
	omp_init_lock(&map->lock);
#endif
	return map;
}

// hash code from java's hashmap
int hash_int(int h) {
	h += ~(h << 9);
	h ^= (h >> 14) | (h << 18);
	h += (h << 4);
	h ^= (h >> 10) | (h << 22);
	return h;
}

int hash(coord_t *bits, int num_bits, int capacity) {
	int hash = 0;
	for (int i=0; i < num_bits; i++)
		hash += hash_int(bits[i].x << 16 | bits[i].y);
	if (hash < 0)
		hash *= -1;
	return hash % capacity;
}

// also acts as contains
bool put(hashmap_t *map, coord_t *bits, int num_bits) {
	int index = hash(bits, num_bits, map->capacity);
#if PARALLEL
	omp_set_lock(&map->lock);
#endif
	entry_t *entry = map->entries[index];
	int length = 0;
	while (entry) {
		if (coord_set_equal(entry->bits, bits, num_bits)) {
#if PARALLEL
			omp_unset_lock(&map->lock);
#endif
			return true;
		}
		entry = entry->next;
		length++;
	}
	map->size++;
	if (length > map->maxlen)
		map->maxlen = length + 1;
	// ok, really have to add it now
	entry = malloc(sizeof(entry_t));
#if FREE_MORE_MEMORY
	entry->bits = malloc(num_bits * sizeof(entry->bits));
	memcpy(entry->bits, bits, num_bits * sizeof(coord_t));
#else
	entry->bits = bits;
#endif
	entry->next = map->entries[index];
	map->entries[index] = entry;
#if PARALLEL
	omp_unset_lock(&map->lock);
#endif
	return false;
}

bool contains(hashmap_t *map, coord_t *bits, int num_bits) {
	int index = hash(bits, num_bits, map->capacity);
#if PARALLEL
	omp_set_lock(&map->lock);
#endif
	entry_t *entry = map->entries[index];
	while (entry) {
		if (coord_set_equal(entry->bits, bits, num_bits)) {
#if PARALLEL
			omp_unset_lock(&map->lock);
#endif
			return true;
		}
		entry = entry->next;
	}
#if PARALLEL
	omp_unset_lock(&map->lock);
#endif
	return false;
}

#endif
