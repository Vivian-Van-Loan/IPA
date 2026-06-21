#include "general_types.h"

void alloc_str_free(alloc_str* str) {
    free(*str);
}
