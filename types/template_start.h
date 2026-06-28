#ifndef TEMPLATE_IS_KV
    #ifndef TEMPLATE_TYPE
    #error "TEMPLATE_TYPE *must* be defined as a single token"
    #endif
#else
    #if !defined(TEMPLATE_TYPE_K) || !defined(TEMPLATE_TYPE_V)
    #error "TEMPLATE_TYPE_K & TEMPLATE_TYPE_V *must* be defined as a single token"
    #endif
#endif

#define TEMPLATE_CAT_(name, type) name ## $ ## type ## $
#define TEMPLATE_CAT(name, type) TEMPLATE_CAT_(name, type)
#define TEMPLATE_STRUCT TEMPLATE_CAT(TEMPLATE_PREFIX, TEMPLATE_TYPE)

#define TEMPLATE_CAT_KV_(name, type_k, type_v) name ## $ ## type_k ## $ ## type_v ## $
#define TEMPLATE_CAT_KV(name, type_k, type_v) TEMPLATE_CAT_KV_(name, type_k, type_v)
#define TEMPLATE_STRUCT_KV TEMPLATE_CAT_KV(TEMPLATE_PREFIX, TEMPLATE_TYPE_K, TEMPLATE_TYPE_V)

#define TEMPLATE_FUNC__(prefix, suffix) prefix ## _ ## suffix
#define TEMPLATE_FUNC_(prefix, suffix) TEMPLATE_FUNC__(prefix, suffix)
#define TEMPLATE_FUNC(suffix) TEMPLATE_FUNC_(TEMPLATE_STRUCT, suffix)

#define TEMPLATE_FUNC_KV(suffix) TEMPLATE_FUNC_(TEMPLATE_STRUCT_KV, suffix)

#ifndef TEMPLATE_IMPLEMENTATION
    #define TEMPLATE_INLINE inline
#else
    #define TEMPLATE_INLINE
#endif
