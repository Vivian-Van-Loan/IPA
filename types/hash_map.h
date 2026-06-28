#define TEMPLATE_IS_KV
#define TEMPLATE_PREFIX hash_map
#include "template_start.h"

#ifndef TEMPLATE_HASH_FUNC
#define TEMPLATE_HASH_FUNC auto_hash
// #error "You must define TEMPLATE_HASH_FUNC"
#endif

#ifndef TEMPLATE_EQUAL_FUNC
#error "You must define TEMPLATE_EQUAL_FUNC as something taking 2 parameters and returning T or F, even \"#define TEMPLATE_EQUAL_FUNC(lhs, rhs) ((lhs) == (rhs))\" is fine"
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "hash.h"

#define TEMPLATE_HASH_ENTRY TEMPLATE_CAT_KV(hash_map_entry, TEMPLATE_TYPE_K, TEMPLATE_TYPE_V)

typedef struct TEMPLATE_HASH_ENTRY {
    TEMPLATE_TYPE_K key;
    TEMPLATE_TYPE_V value;
    hash_t hash;
    struct TEMPLATE_HASH_ENTRY* next; //tempted to change this to a balanced tree, but probably not needed for our purposes
} TEMPLATE_HASH_ENTRY;

typedef struct TEMPLATE_STRUCT_KV {
    size_t num_items;
    size_t num_buckets;
    TEMPLATE_HASH_ENTRY** buckets;
} TEMPLATE_STRUCT_KV;

TEMPLATE_INLINE TEMPLATE_STRUCT_KV TEMPLATE_FUNC_KV(new)();
TEMPLATE_INLINE void TEMPLATE_FUNC_KV(free)(TEMPLATE_STRUCT_KV* map);
TEMPLATE_INLINE void TEMPLATE_FUNC_KV(free_callback)(TEMPLATE_STRUCT_KV* map, void (*free_func_k)(TEMPLATE_TYPE_K*), void (*free_func_v)(TEMPLATE_TYPE_V*));
TEMPLATE_INLINE void TEMPLATE_FUNC_KV(resize)(TEMPLATE_STRUCT_KV* map, size_t new_buckets);
TEMPLATE_INLINE void TEMPLATE_FUNC_KV(increase)(TEMPLATE_STRUCT_KV* map);
TEMPLATE_INLINE void TEMPLATE_FUNC_KV(decrease)(TEMPLATE_STRUCT_KV* map);
TEMPLATE_INLINE void TEMPLATE_FUNC_KV(eval)(TEMPLATE_STRUCT_KV* map);
TEMPLATE_INLINE TEMPLATE_TYPE_V* TEMPLATE_FUNC_KV(add)(TEMPLATE_STRUCT_KV* map, TEMPLATE_TYPE_K key, TEMPLATE_TYPE_V value);
TEMPLATE_INLINE TEMPLATE_TYPE_V* TEMPLATE_FUNC_KV(remove)(TEMPLATE_STRUCT_KV* map, TEMPLATE_TYPE_K key);
TEMPLATE_INLINE TEMPLATE_TYPE_V* TEMPLATE_FUNC_KV(remove_callback)(TEMPLATE_STRUCT_KV* map, TEMPLATE_TYPE_K key, void (*free_func_k)(TEMPLATE_TYPE_K*));
TEMPLATE_INLINE TEMPLATE_TYPE_V* TEMPLATE_FUNC_KV(find)(TEMPLATE_STRUCT_KV* map, TEMPLATE_TYPE_K key);

TEMPLATE_INLINE TEMPLATE_STRUCT_KV TEMPLATE_FUNC_KV(new)() {
    return (TEMPLATE_STRUCT_KV) {.num_items = 0, .num_buckets = 0, .buckets = nullptr};
}

TEMPLATE_INLINE void TEMPLATE_FUNC_KV(free)(TEMPLATE_STRUCT_KV* map) {
    TEMPLATE_FUNC_KV(free_callback)(map, nullptr, nullptr);
}

TEMPLATE_INLINE void TEMPLATE_FUNC_KV(free_callback)(TEMPLATE_STRUCT_KV* map, void (*free_func_k)(TEMPLATE_TYPE_K*), void (*free_func_v)(TEMPLATE_TYPE_V*)) {
    for (size_t i = 0; i < map->num_buckets; i++) {
        TEMPLATE_HASH_ENTRY* entry = map->buckets[i];
        while (entry) {
            TEMPLATE_HASH_ENTRY* next = entry->next;
            if (free_func_k) {
                free_func_k(&entry->key);
            }
            if (free_func_v) {
                free_func_v(&entry->value);
            }
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
}

TEMPLATE_INLINE void TEMPLATE_FUNC_KV(resize)(TEMPLATE_STRUCT_KV* map, size_t new_buckets) {
    if (new_buckets == map->num_buckets || new_buckets == 0) { //I am NOT dealing with 0 cases
        return;
    }

    TEMPLATE_HASH_ENTRY** new_buckets_ptr = calloc(new_buckets, sizeof(TEMPLATE_HASH_ENTRY*));
    if (map->buckets) { //should be unneeded but I was getting clang warnings and that frightens me
        for (size_t i = 0; i < map->num_buckets; i++) {
            TEMPLATE_HASH_ENTRY* entry = map->buckets[i];
            while (entry) {
                TEMPLATE_HASH_ENTRY* next = entry->next;
                size_t bucket_idx = entry->hash % new_buckets;
                entry->next = new_buckets_ptr[bucket_idx]; //Works but reverses the LL order if same bucket (should not matter)
                new_buckets_ptr[bucket_idx] = entry;
                entry = next;
            }
        }
        free(map->buckets);
    }

    map->buckets = new_buckets_ptr;
    map->num_buckets = new_buckets;
}

TEMPLATE_INLINE void TEMPLATE_FUNC_KV(increase)(TEMPLATE_STRUCT_KV* map) {
    size_t new_buckets = map->num_buckets * 2;
    if (new_buckets == 0) {
        new_buckets = 8;
    }
    TEMPLATE_FUNC_KV(resize)(map, new_buckets);
}

TEMPLATE_INLINE void TEMPLATE_FUNC_KV(decrease)(TEMPLATE_STRUCT_KV* map) {
    size_t new_buckets = map->num_buckets / 2;
    if (new_buckets <= 2) {
        new_buckets = 2; //we are not going below 2, literally what is the point if you do?
    }
    TEMPLATE_FUNC_KV(resize)(map, new_buckets);
}

TEMPLATE_INLINE void TEMPLATE_FUNC_KV(eval)(TEMPLATE_STRUCT_KV* map) {
    float load_factor = (float) map->num_items / (float) map->num_buckets;
    if (load_factor >= 1.5) {
        TEMPLATE_FUNC_KV(increase)(map);
    } else if (load_factor < 0.25) {
        TEMPLATE_FUNC_KV(decrease)(map);
    }
}

TEMPLATE_INLINE TEMPLATE_TYPE_V* TEMPLATE_FUNC_KV(add)(TEMPLATE_STRUCT_KV* map, TEMPLATE_TYPE_K key, TEMPLATE_TYPE_V value) {
    if (map->num_buckets == 0) {
        TEMPLATE_FUNC_KV(increase)(map);
    }
    hash_t hash = auto_hash(key);
    size_t bucket_idx = hash % map->num_buckets;
    TEMPLATE_HASH_ENTRY* entry = map->buckets[bucket_idx];
    if (entry) {
        while (entry->next) {
            entry = entry->next;
        }
        entry->next = (TEMPLATE_HASH_ENTRY*) malloc(sizeof(TEMPLATE_HASH_ENTRY));
        entry = entry->next;
    } else {
        entry = (TEMPLATE_HASH_ENTRY*) malloc(sizeof(TEMPLATE_HASH_ENTRY));
    }
#ifdef TEMPLATE_TYPE_K_IS_ARRAY //be VERY careful with this as it requires an ACTUAL ARRAY and NOT A POINTER/DECAYED ARRAY
    memcpy(entry->key, key, sizeof(TEMPLATE_TYPE_K));
#else
    entry->key = key;
#endif

#ifdef TEMPLATE_TYPE_V_IS_ARRAY
    memcpy(entry->value, value, sizeof(TEMPLATE_TYPE_V));
#else
    entry->value = value;
#endif
    entry->hash = hash;
    entry->next = nullptr;
    map->num_items++;

    TEMPLATE_FUNC_KV(eval)(map);

    return &entry->value;
}

TEMPLATE_INLINE TEMPLATE_TYPE_V* TEMPLATE_FUNC_KV(remove)(TEMPLATE_STRUCT_KV* map, TEMPLATE_TYPE_K key) {
    return TEMPLATE_FUNC_KV(remove_callback)(map, key, nullptr);
}

TEMPLATE_INLINE TEMPLATE_TYPE_V* TEMPLATE_FUNC_KV(remove_callback)(TEMPLATE_STRUCT_KV* map, TEMPLATE_TYPE_K key, void (*free_func_k)(TEMPLATE_TYPE_K*)) {
    hash_t hash = TEMPLATE_HASH_FUNC(key);
    size_t bucket_idx = hash % map->num_buckets;
    TEMPLATE_HASH_ENTRY* entry = map->buckets[bucket_idx];
    TEMPLATE_HASH_ENTRY* prev = nullptr;
    while (entry) {
        if (entry->hash == hash && TEMPLATE_EQUAL_FUNC(key, entry->key)) {
            if (prev) {
                prev->next = entry->next;
            } else {
                map->buckets[bucket_idx] = entry->next;
            }
            break;
        }
        prev = entry;
        entry = entry->next;
    }
    if (entry && free_func_k) {
        free_func_k(&entry->key);
    }
    map->num_items--;
    TEMPLATE_FUNC_KV(eval)(map);
    return entry ? &entry->value : nullptr;
}

TEMPLATE_INLINE TEMPLATE_TYPE_V* TEMPLATE_FUNC_KV(find)(TEMPLATE_STRUCT_KV* map, TEMPLATE_TYPE_K key) {
    hash_t hash = TEMPLATE_HASH_FUNC(key);
    size_t bucket_idx = hash % map->num_buckets;
    TEMPLATE_HASH_ENTRY* entry = map->buckets[bucket_idx];
    while (entry) {
        if (entry->hash == hash && TEMPLATE_EQUAL_FUNC(key, entry->key)) {
            return &entry->value;
        }
        entry = entry->next;
    }
    return nullptr;
}

#undef TEMPLATE_HASH_FUNC
#undef TEMPLATE_EQUAL_FUNC

#include "template_end.h"
