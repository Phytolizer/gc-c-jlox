#include <private/token_type.h>

const char* TOKEN_TYPE_STRINGS[] = {
#define VARIANT(x) #x,
#include "token_type.inl"
#undef VARIANT
};
