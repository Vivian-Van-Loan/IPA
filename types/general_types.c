#include "general_types.h"

void alloc_str_free(alloc_str str) {
    free(str);
}

void alloc_void_free(alloc_void data) {
    free(data);
}
