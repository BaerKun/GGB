#include "utils.h"
#include "geom_errors.h"
#include <string.h>

typedef struct pcg_state_setseq_64 {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

static pcg32_random_t pcg32_global = {0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL};

uint32_t random32(void) {
    const uint64_t oldstate = pcg32_global.state;
    pcg32_global.state = oldstate * 6364136223846793005ULL + pcg32_global.inc;
    const uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    const uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void randomSeed(const uint64_t initstate, const uint64_t initseq) {
    pcg32_global.state = 0U;
    pcg32_global.inc = (initseq << 1u) | 1u;
    random32();
    pcg32_global.state += initstate;
    random32();
}

const char *invalidColor() {
    static const char *tips = "Please use hexadecimal.";
    static char error[40] = {0};

    if (*error == 0)
        strcpy(error, invalidArg("color", tips));
    return error;
}

uint64_t strhash64(const char *str) {
    uint64_t hash = 0;
    char *p = (char *) &hash;

    for (int i = 0; i < 8 && *str != 0; ++i) {
        *p = *str;
        ++p, ++str;
    }
    return hash;
}

int strtobool(const char *str, const char **endptr) {
    switch (strhash64(str)) {
        case STR_HASH64('t', 'r', 'u', 'e', 0, 0, 0, 0):
            *endptr = str + 4;
            return 1;
        case STR_HASH64('f', 'a', 'l', 's', 'e', 0, 0, 0):
            *endptr = str + 5;
            return 0;
        default:
            *endptr = str;
            return 0;
    }
}
