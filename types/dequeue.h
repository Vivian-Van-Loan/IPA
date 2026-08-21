#define TEMPLATE_PREFIX dequeue
#include "template_start.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct TEMPLATE_STRUCT {
    size_t count;
    size_t capacity;
    TEMPLATE_TYPE* data;
    TEMPLATE_TYPE* front;
} TEMPLATE_STRUCT;

TEMPLATE_INLINE TEMPLATE_STRUCT TEMPLATE_FUNC(new)();
TEMPLATE_INLINE void TEMPLATE_FUNC(grow)(TEMPLATE_STRUCT* deq);
TEMPLATE_INLINE void TEMPLATE_FUNC(free)(TEMPLATE_STRUCT* deq);
TEMPLATE_INLINE void TEMPLATE_FUNC(free_callback)(TEMPLATE_STRUCT* deq, void (*free_func)(TEMPLATE_TYPE*));
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_empty)(TEMPLATE_STRUCT* deq);
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push)(TEMPLATE_STRUCT* deq, TEMPLATE_TYPE value);
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_back_empty)(TEMPLATE_STRUCT* deq);
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_back)(TEMPLATE_STRUCT* deq, TEMPLATE_TYPE value);
TEMPLATE_INLINE void TEMPLATE_FUNC(remove)(TEMPLATE_STRUCT* deq, size_t idx);
TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(replace)(TEMPLATE_STRUCT* deq, size_t idx, TEMPLATE_TYPE value);
TEMPLATE_INLINE void TEMPLATE_FUNC(resize_keep_front)(TEMPLATE_STRUCT* deq, size_t new_capacity);
TEMPLATE_INLINE void TEMPLATE_FUNC(resize_keep_back)(TEMPLATE_STRUCT* deq, size_t new_capacity);
TEMPLATE_INLINE void TEMPLATE_FUNC(shrink)(TEMPLATE_STRUCT* deq);

TEMPLATE_INLINE TEMPLATE_STRUCT TEMPLATE_FUNC(new)() {
    TEMPLATE_STRUCT deq = {
        .count = 0,
        .capacity = 0,
        .data = nullptr,
        .front = nullptr
    };
    return deq;
}

TEMPLATE_INLINE void TEMPLATE_FUNC(grow)(TEMPLATE_STRUCT* deq) {
    size_t new_capacity = deq->capacity * 2;
    if (new_capacity == 0) {
        new_capacity = 4;
    }
    TEMPLATE_FUNC(resize_keep_front)(deq, new_capacity);
}

TEMPLATE_INLINE void TEMPLATE_FUNC(free)(TEMPLATE_STRUCT* deq) {
    free(deq->front);
    deq->front = nullptr;
    deq->data = nullptr;
    deq->count = 0;
    deq->capacity = 0;
}

TEMPLATE_INLINE void TEMPLATE_FUNC(free_callback)(TEMPLATE_STRUCT* deq, void (*free_func)(TEMPLATE_TYPE*)) {
    for (size_t i = 0; i < deq->count; i++) {
        free_func(deq->data + i);
    }
    TEMPLATE_FUNC(free)(deq);
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_empty)(TEMPLATE_STRUCT* deq) {
    if (deq->data == deq->front) {
        TEMPLATE_FUNC(grow)(deq);
    }
    deq->data--;
    deq->count++;
    return &deq->data[0];
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push)(TEMPLATE_STRUCT* deq, TEMPLATE_TYPE value) {
    TEMPLATE_TYPE* new = TEMPLATE_FUNC(push_empty)(deq);
#ifdef TEMPLATE_TYPE_IS_ARRAY
    memcpy(*new, value, sizeof(TEMPLATE_TYPE));
#else
    *new = value;
#endif
    return new;
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_back_empty)(TEMPLATE_STRUCT* deq) {
    if (deq->count == deq->capacity) {
        TEMPLATE_FUNC(grow)(deq);
    }
    deq->count++;
    return &deq->data[deq->count - 1];
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(push_back)(TEMPLATE_STRUCT* deq, TEMPLATE_TYPE value) {
    TEMPLATE_TYPE* new = TEMPLATE_FUNC(push_back_empty)(deq);
#ifdef TEMPLATE_TYPE_IS_ARRAY
    memcpy(*new, value, sizeof(TEMPLATE_TYPE));
#else
    *new = value;
#endif
    return new;
}

TEMPLATE_INLINE void TEMPLATE_FUNC(remove)(TEMPLATE_STRUCT* deq, size_t idx) {
    if (deq->count == 0) {
        return;
    }
    if (idx == 0) {
        deq->data++;
        deq->count--;
    } else {
        //todo: so this does mean that any removals have capacity shifted onto the end
        // ideal would be to check which side of the middle the element is on and shift relative to that
        // so do that in future I suppose
        memmove(deq->data + idx - 1, deq->data + idx, (deq->count - idx) * sizeof(TEMPLATE_TYPE));
        deq->count--;
    }
}

TEMPLATE_INLINE TEMPLATE_TYPE* TEMPLATE_FUNC(replace)(TEMPLATE_STRUCT* deq, size_t idx, TEMPLATE_TYPE value) {
    TEMPLATE_TYPE* entry = deq->data + idx;

#ifdef TEMPLATE_TYPE_IS_ARRAY
    memcpy(*entry, value, sizeof(TEMPLATE_TYPE));
#else
    *entry = value;
#endif
}

TEMPLATE_INLINE void TEMPLATE_FUNC(resize_keep_front)(TEMPLATE_STRUCT* deq, size_t new_capacity) {
    if (new_capacity < deq->count) {
        deq->count = new_capacity;
    }

    if (new_capacity == 0) {
        TEMPLATE_FUNC(free)(deq);
        return;
    }
    ptrdiff_t data_offset = 0;
    if (deq->count != 0) {
        data_offset = deq->data - deq->front;
    }
    size_t mid_start = (new_capacity - deq->count) / 2;
    deq->front = realloc(deq->front, new_capacity * sizeof(TEMPLATE_TYPE));
    memmove(deq->front + mid_start, deq->front + data_offset, deq->count * sizeof(TEMPLATE_TYPE));

    deq->data = deq->front + mid_start;
    deq->capacity = new_capacity;
}

TEMPLATE_INLINE void TEMPLATE_FUNC(resize_keep_back)(TEMPLATE_STRUCT* deq, size_t new_capacity) { //todo: consider merging with above
    if (new_capacity == 0) {
        TEMPLATE_FUNC(free)(deq);
        deq->count = 0;
        return;
    }

    size_t new_count = deq->count;
    if (new_capacity < new_count) {
        new_count = new_capacity;
    }
    size_t old_start = deq->count - new_count;
    size_t mid_start = (new_capacity - new_count) / 2;

    ptrdiff_t data_offset = 0;
    if (deq->count != 0) {
        data_offset = deq->data - deq->front;
    }

    deq->front = realloc(deq->front, new_capacity * sizeof(TEMPLATE_TYPE));

    memmove(deq->front + mid_start, deq->front + data_offset + old_start, new_count * sizeof(TEMPLATE_TYPE));

    deq->data = deq->front + mid_start;
    deq->capacity = new_capacity;
    deq->count = new_count;
}

TEMPLATE_INLINE void TEMPLATE_FUNC(shrink)(TEMPLATE_STRUCT* deq) {
    TEMPLATE_FUNC(resize_keep_front)(deq, deq->count);
}
