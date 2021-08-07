#include <private/hash/fnv.h>
#include <stdint.h>

#define FNV_OFFSET_BASIS 0xCBF29CE484222325UL
#define FNV_PRIME 0x100000001B3UL

uint64_t hash_fnv1(const void* vdata, size_t len)
{
  const uint8_t* data = vdata;
  uint64_t hash = FNV_OFFSET_BASIS;
  for (size_t i = 0; i < len; ++i) {
    hash *= FNV_PRIME;
    hash ^= data[i];
  }
  return hash;
}

uint64_t hash_fnv1a(const void* vdata, size_t len)
{
  const uint8_t* data = vdata;
  uint64_t hash = FNV_OFFSET_BASIS;
  for (size_t i = 0; i < len; ++i) {
    hash ^= data[i];
    hash *= FNV_PRIME;
  }
  return hash;
}
