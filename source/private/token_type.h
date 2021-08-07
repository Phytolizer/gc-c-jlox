#pragma once

enum token_type
{
#define VARIANT(x) x,
#include "token_type.inl"
#undef VARIANT
};

extern const char* TOKEN_TYPE_STRINGS[];
