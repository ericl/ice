#ifndef HASHMAP_H
#define HASHMAP_H

#include <state.h>
#include <common.h>

#define HASHMAP_DEFAULT_CAPACITY 93563 // prime

typedef struct entry {
	coord_t *bits;
	struct entry *next;
} entry_t;

typedef struct hashmap {
	entry_t **entries;
	int capacity;
	int size;
	int maxlen;
} hashmap_t;

hashmap_t *create_hashmap() {
	hashmap_t *map = malloc(sizeof(hashmap_t));
	map->size = 0;
	map->maxlen = 0;
	map->capacity = HASHMAP_DEFAULT_CAPACITY;
	map->entries = calloc(HASHMAP_DEFAULT_CAPACITY, sizeof(entry_t));
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

int hash(coord_t *bits, int num_bits) {
	int hash = 0;
	for (int i=0; i < num_bits; i++)
		hash += hash_int(bits[i].x << 16 | bits[i].y);
	return hash;
}

// also acts as contains
bool put(hashmap_t *map, coord_t *bits, int num_bits) {
	int index = hash(bits, num_bits);
	if (index < 0)
		index *= -1;
	index %= map->capacity;
	entry_t *entry = map->entries[index];
	int length = 0;
	while (entry) {
		if (coord_set_equal(entry->bits, bits, num_bits))
			return true;
		entry = entry->next;
		length++;
	}
	map->size++;
	if (length > map->maxlen)
		map->maxlen = length + 1;
	// ok, really have to add it now
	entry = malloc(sizeof(entry_t));
	entry->bits = bits;
	entry->next = map->entries[index];
	map->entries[index] = entry;
	return false;
}

bool contains(hashmap_t *map, coord_t *bits, int num_bits) {
	int index = hash(bits, num_bits) % map->capacity;
	entry_t *entry = map->entries[index];
	while (entry) {
		if (coord_set_equal(entry->bits, bits, num_bits))
			return true;
		entry = entry->next;
	}
	return false;
}

#endif
