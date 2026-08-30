#if !defined(TEMPLATE_IS_KV) && !defined(TEMPLATE_IS_3)
    #ifndef TEMPLATE_TYPE
    #error "TEMPLATE_TYPE *must* be defined as a single token"
    #endif
#elif defined(TEMPLATE_IS_KV)
    #if !defined(TEMPLATE_TYPE_K) || !defined(TEMPLATE_TYPE_V)
    #error "TEMPLATE_TYPE_K & TEMPLATE_TYPE_V *must* be defined as a single token"
    #endif
#elif defined(TEMPLATE_IS_3)
    #if !defined(TEMPLATE_TYPE_1) || !defined(TEMPLATE_TYPE_2) || !defined(TEMPLATE_TYPE_3)
    #error "TEMPLATE_TYPE_1, TEMPLATE_TYPE_2, & TEMPLATE_TYPE_3 *must* be defined as a single token"
    #endif
#endif

#define TEMPLATE_CAT_(name, type) name ## $ ## type ## $
#define TEMPLATE_CAT(name, type) TEMPLATE_CAT_(name, type)
#define TEMPLATE_STRUCT TEMPLATE_CAT(TEMPLATE_PREFIX, TEMPLATE_TYPE)

#define TEMPLATE_CAT_KV_(name, type_k, type_v) name ## $ ## type_k ## $ ## type_v ## $
#define TEMPLATE_CAT_KV(name, type_k, type_v) TEMPLATE_CAT_KV_(name, type_k, type_v)
#define TEMPLATE_STRUCT_KV TEMPLATE_CAT_KV(TEMPLATE_PREFIX, TEMPLATE_TYPE_K, TEMPLATE_TYPE_V)

#define TEMPLATE_CAT_3_(name, type_1, type_2, type_3) name ## $ ## type_1 ## $ ## type_2 ## $ ## type_3 ## $
#define TEMPLATE_CAT_3(name, type_1, type_2, type_3) TEMPLATE_CAT_3_(name, type_1, type_2, type_3)
#define TEMPLATE_STRUCT_3 TEMPLATE_CAT_3(TEMPLATE_PREFIX, TEMPLATE_TYPE_1, TEMPLATE_TYPE_2, TEMPLATE_TYPE_3)

#define TEMPLATE_FUNC__(prefix, suffix) prefix ## _ ## suffix
#define TEMPLATE_FUNC_(prefix, suffix) TEMPLATE_FUNC__(prefix, suffix)
#define TEMPLATE_FUNC(suffix) TEMPLATE_FUNC_(TEMPLATE_STRUCT, suffix)

#define TEMPLATE_FUNC_KV(suffix) TEMPLATE_FUNC_(TEMPLATE_STRUCT_KV, suffix)

#define TEMPLATE_FUNC_3(suffix) TEMPLATE_FUNC_(TEMPLATE_STRUCT_3, suffix)

#ifndef TEMPLATE_IMPLEMENTATION
    #define TEMPLATE_INLINE inline
#else
    #define TEMPLATE_INLINE
#endif
