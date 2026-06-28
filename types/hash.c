#include "hash.h"

hash_t hash_value(void const* data_void, size_t size) {
    unsigned char const* data = data_void;
    hash_t hash = FNV_OFFSET;
    for (size_t i = 0; i < size; i++) {
        hash = FNV_PRIME * (hash ^ data[i]);
    }
    return hash;
}

hash_t hash_string(char const* data) {
    hash_t hash = FNV_OFFSET;
    while (*data) {
        hash = FNV_PRIME * (hash ^ *data);
        data++;
    }
    return hash;
}

hash_t hash_bool(bool b, size_t size) {
    uint8_t val = b;
    return hash_value(&val, sizeof(val));
}