#ifndef MATRIX_CLIENT_HASH_H
#define MATRIX_CLIENT_HASH_H

#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <stdint-gcc.h> //compiler is being weird and won't define SIZE_WIDTH in stdint.h as per normal, whatever

#if SIZE_WIDTH == 32
typedef uint32_t hash_t;
constexpr hash_t FNV_PRIME = 0x01000193;
constexpr hash_t FNV_OFFSET = 0x811c9dc5;
#elif SIZE_WIDTH == 64
typedef uint64_t hash_t;
constexpr hash_t FNV_PRIME = 0x00000100000001b3;
constexpr hash_t FNV_OFFSET = 0xcbf29ce484222325;
#else
#error "Unsupported processor bitwidth"
#endif

hash_t hash_value(void const* data_void, size_t size);
hash_t hash_string(char const* data);

hash_t hash_bool(bool b, size_t size);

#define auto_hash(x) _Generic((x), \
                    char*: hash_string, \
                    char const*: hash_string, \
                    unsigned char*: hash_string, \
                    unsigned char const*: hash_string, \
                    signed char*: hash_string, \
                    signed char const*: hash_string, \
                    \
                    bool: hash_bool, \
                    int8_t: hash_value, \
                    uint8_t: hash_value, \
                    int16_t: hash_value, \
                    uint16_t: hash_value, \
                    int32_t: hash_value, \
                    uint32_t: hash_value, \
                    int64_t: hash_value, \
                    uint64_t: hash_value, \
                    char: hash_value \
                    )(x)

#endif //MATRIX_CLIENT_HASH_H
