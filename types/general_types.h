#ifndef MATRIX_CLIENT_GENERAL_TYPES_H
#define MATRIX_CLIENT_GENERAL_TYPES_H

typedef char* alloc_str;
#define TEMPLATE_TYPE alloc_str
#include "vector.h"

void alloc_str_free(alloc_str* str);

#endif //MATRIX_CLIENT_GENERAL_TYPES_H
