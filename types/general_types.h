#ifndef MATRIX_CLIENT_GENERAL_TYPES_H
#define MATRIX_CLIENT_GENERAL_TYPES_H

typedef char* str;
typedef char const* str_const;
typedef char* alloc_str;
#define TEMPLATE_TYPE alloc_str
#include "vector.h"

void alloc_str_free(alloc_str* str);

typedef void* void_p;
typedef void const* void_const_p;
typedef void* alloc_void;
#define TEMPLATE_TYPE alloc_void
#include "vector.h"

void alloc_void_free(alloc_void* void_p);

#define TEMPLATE_TYPE_K alloc_str
#define TEMPLATE_TYPE_V int
#define TEMPLATE_EQUAL_FUNC(lhs, rhs) (strcmp((lhs), (rhs)) == 0)
#include "hash_map.h"

#endif //MATRIX_CLIENT_GENERAL_TYPES_H
