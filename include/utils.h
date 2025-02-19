#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define STR_HASH64(c1, c2, c3, c4, c5, c6, c7, c8)( \
(uint64_t) (c8) << 56 | (uint64_t) (c7) << 48 | \
(uint64_t) (c6) << 40 | (uint64_t) (c5) << 32 | \
(uint64_t) (c4) << 24 | (uint64_t) (c3) << 16 | \
(uint64_t) (c2) << 8 | (uint64_t) (c1))

uint64_t strhash64(const char *str);

int strtobool(const char *str, const char **endptr);

uint32_t random32();

void randomSeed(uint64_t initstate, uint64_t initseq);

const char *invalidColor();

#endif //UTILS_H
