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

// #ifndef TEMPLATE_IMPLEMENTATION
TEMPLATE_INLINE TEMPLATE_STRUCT TEMPLATE_FUNC(new)();
TEMPLATE_INLINE void TEMPLATE_FUNC(double)(TEMPLATE_STRUCT* vec);
TEMPLATE_INLINE void TEMPLATE_FUNC(free)(TEMPLATE_STRUCT* vec);
TEMPLATE_INLINE void TEMPLATE_FUNC(free_callback)(TEMPLATE_STRUCT* vec, void (*free_func)(TEMPLATE_TYPE*));
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_back_empty)(TEMPLATE_STRUCT* vec);
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_back)(TEMPLATE_STRUCT* vec, TEMPLATE_TYPE value);
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(insert_empty_at)(TEMPLATE_STRUCT* vec, size_t idx);
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(insert_at)(TEMPLATE_STRUCT* vec, size_t idx, TEMPLATE_TYPE value);
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(get)(TEMPLATE_STRUCT* vec, size_t idx);
TEMPLATE_INLINE void TEMPLATE_FUNC(remove)(TEMPLATE_STRUCT* vec, size_t idx);
TEMPLATE_INLINE void TEMPLATE_FUNC(resize)(TEMPLATE_STRUCT* vec, size_t new_capacity);
TEMPLATE_INLINE void TEMPLATE_FUNC(shrink)(TEMPLATE_STRUCT* vec);
// #endif

// #ifdef TEMPLATE_IMPLEMENTATION
TEMPLATE_INLINE TEMPLATE_STRUCT TEMPLATE_FUNC(new)() {
    TEMPLATE_STRUCT vec = {
        .count = 0,
        .capacity = 0,
        .data = nullptr
    };
    return vec;
}

TEMPLATE_INLINE void TEMPLATE_FUNC(double)(TEMPLATE_STRUCT* vec) {
    size_t new_capacity = vec->capacity * 2;
    if (new_capacity == 0) {
        new_capacity = 1;
    }
    TEMPLATE_FUNC(resize)(vec, new_capacity);
}

TEMPLATE_INLINE void TEMPLATE_FUNC(free)(TEMPLATE_STRUCT* vec) {
    free(vec->data);
    vec->data = nullptr;
    vec->count = 0;
    vec->capacity = 0;
}

TEMPLATE_INLINE void TEMPLATE_FUNC(free_callback)(TEMPLATE_STRUCT* vec, void (*free_func)(TEMPLATE_TYPE*)) {
    for (size_t i = 0; i < vec->count; i++) {
        free_func(vec->data + i);
    }
    TEMPLATE_FUNC(free)(vec);
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_back_empty)(TEMPLATE_STRUCT* vec) {
    if (vec->count == vec->capacity) {
        TEMPLATE_FUNC(double)(vec);
    }
    vec->count++;
    return &vec->data[vec->count - 1];
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_back)(TEMPLATE_STRUCT* vec, TEMPLATE_TYPE value) {
    TEMPLATE_TYPE* new = TEMPLATE_FUNC(push_back_empty)(vec);
#ifdef TEMPLATE_TYPE_IS_ARRAY
    memcpy(*new, value, sizeof(TEMPLATE_TYPE));
#else
    *new = value;
#endif
    return new;
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(insert_empty_at)(TEMPLATE_STRUCT* vec, size_t idx) {
    if (vec->count == vec->capacity) {
        TEMPLATE_FUNC(double)(vec);
    }
    memmove(vec->data + idx + 1, vec->data + idx, (vec->count - idx) * sizeof(TEMPLATE_TYPE));
    vec->count++;
    return &vec->data[idx];
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(insert_at)(TEMPLATE_STRUCT* vec, size_t idx, TEMPLATE_TYPE value) {
    TEMPLATE_TYPE* new = TEMPLATE_FUNC(insert_empty_at)(vec, idx);
#ifdef TEMPLATE_TYPE_IS_ARRAY
    memcpy(*new, value, sizeof(TEMPLATE_TYPE));
#else
    *new = value;
#endif
    return new;
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(get)(TEMPLATE_STRUCT* vec, size_t idx) {
    if (idx >= vec->count) {
        return nullptr;
    }
    return vec->data + idx;
}

TEMPLATE_INLINE void TEMPLATE_FUNC(remove)(TEMPLATE_STRUCT* vec, size_t idx) {
    memcpy(vec->data + idx, vec->data + (idx + 1), (vec->count - idx - 1) * sizeof(TEMPLATE_TYPE));
    vec->count--;
}

TEMPLATE_INLINE void TEMPLATE_FUNC(resize)(TEMPLATE_STRUCT* vec, size_t new_capacity) {
    vec->capacity = new_capacity;
    if (vec->capacity < vec->count) {
        vec->count = vec->capacity;
    }
    if (vec->capacity == 0) {
        TEMPLATE_FUNC(free)(vec);
    } else {
        vec->data = realloc(vec->data, vec->capacity * sizeof(TEMPLATE_TYPE));
    }
}

TEMPLATE_INLINE void TEMPLATE_FUNC(shrink)(TEMPLATE_STRUCT* vec) {
    TEMPLATE_FUNC(resize)(vec, vec->count);
}
// #endif

#include "template_end.h"
