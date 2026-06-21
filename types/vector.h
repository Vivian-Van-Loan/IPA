#define TEMPLATE_PREFIX vector
#include "template_start.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct TEMPLATE_STRUCT {
    // size_t value_size;
    size_t count;
    size_t capacity;
    TEMPLATE_TYPE* data;
} TEMPLATE_STRUCT;

static inline void TEMPLATE_FUNC(resize)(TEMPLATE_STRUCT* vec, size_t new_capacity);

static inline TEMPLATE_STRUCT TEMPLATE_FUNC(new)() {
    TEMPLATE_STRUCT vec = {
        .count = 0,
        .capacity = 0,
        .data = nullptr
    };
    return vec;
}

static inline void TEMPLATE_FUNC(double)(TEMPLATE_STRUCT* vec) {
    size_t new_capacity = vec->capacity * 2;
    if (new_capacity == 0) {
        new_capacity = 1;
    }
    TEMPLATE_FUNC(resize)(vec, new_capacity);
}

static inline void TEMPLATE_FUNC(free)(TEMPLATE_STRUCT* vec) {
    free(vec->data);
    vec->data = nullptr;
    vec->count = 0;
    vec->capacity = 0;
}

static inline void TEMPLATE_FUNC(free_callback)(TEMPLATE_STRUCT* vec, void (*free_func)(TEMPLATE_TYPE*)) {
    for (size_t i = 0; i < vec->count; i++) {
        free_func(vec->data + i);
    }
    TEMPLATE_FUNC(free)(vec);
}

static inline TEMPLATE_TYPE* TEMPLATE_FUNC(push_empty)(TEMPLATE_STRUCT* vec) {
    if (vec->count == vec->capacity) {
        TEMPLATE_FUNC(double)(vec);
    }
    vec->count++;
    return &vec->data[vec->count - 1];
}

static inline TEMPLATE_TYPE* TEMPLATE_FUNC(push)(TEMPLATE_STRUCT* vec, TEMPLATE_TYPE value) {
    TEMPLATE_TYPE* new = TEMPLATE_FUNC(push_empty)(vec);
    *new = value;
    return new;
}

static inline void* TEMPLATE_FUNC(get)(TEMPLATE_STRUCT* vec, size_t idx) {
    if (idx >= vec->count) {
        return nullptr;
    }
    return vec->data + idx;
}

static inline void TEMPLATE_FUNC(remove)(TEMPLATE_STRUCT* vec, size_t idx) {
    memcpy(vec->data + idx, vec->data + (idx + 1), (vec->count - idx - 1) * sizeof(TEMPLATE_TYPE));
    vec->count--;
}

static inline void TEMPLATE_FUNC(resize)(TEMPLATE_STRUCT* vec, size_t new_capacity) {
    vec->capacity = new_capacity;
    if (vec->capacity < vec->count) {
        vec->count = vec->capacity;
    }
    if (vec->capacity == 0) {
        free(vec->data);
        vec->data = nullptr;
    } else {
        vec->data = realloc(vec->data, vec->capacity * sizeof(TEMPLATE_TYPE));
    }
}

static inline void TEMPLATE_FUNC(shrink)(TEMPLATE_STRUCT* vec) {
    TEMPLATE_FUNC(resize)(vec, vec->count);
}

#include "template_end.h"
