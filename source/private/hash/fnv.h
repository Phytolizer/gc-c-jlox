#pragma once

#include <stddef.h>
#include <stdint.h>

uint64_t hash_fnv1(const void* vdata, size_t len);
uint64_t hash_fnv1a(const void* vdata, size_t len);
