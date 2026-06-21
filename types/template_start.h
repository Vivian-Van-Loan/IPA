#ifndef TEMPLATE_TYPE
#error "TEMPLATE_TYPE *must* be defined as a single token"
#endif

#define TEMPLATE_CAT_(name, type) name ## $ ## type ## $
#define TEMPLATE_CAT(name, type) TEMPLATE_CAT_(name, type)
#define TEMPLATE_STRUCT TEMPLATE_CAT(TEMPLATE_PREFIX, TEMPLATE_TYPE)

#define TEMPLATE_FUNC__(prefix, suffix) prefix ## _ ## suffix
#define TEMPLATE_FUNC_(prefix, suffix) TEMPLATE_FUNC__(prefix, suffix)
#define TEMPLATE_FUNC(suffix) TEMPLATE_FUNC_(TEMPLATE_STRUCT, suffix)
