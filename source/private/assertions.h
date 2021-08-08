#pragma once

#include <assert.h>

#define ASSERT_UNREACHABLE() assert(false && "entered unreachable code")
