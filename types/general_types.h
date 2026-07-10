#ifndef MATRIX_CLIENT_GENERAL_TYPES_H
#define MATRIX_CLIENT_GENERAL_TYPES_H

typedef char* alloc_str;
#define TEMPLATE_TYPE alloc_str
#include "vector.h"

typedef void* alloc_void;
#define TEMPLATE_TYPE alloc_void
#include "vector.h"

#endif //MATRIX_CLIENT_GENERAL_TYPES_H
