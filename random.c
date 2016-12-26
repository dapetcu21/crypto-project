#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

inline
uint64_t nextState(uint64_t rngState) {
    uint32_t r0 = 18273 * (uint32_t)((rngState >> 32) & 0xffff) + (uint32_t)(rngState >> 48);
    uint32_t r1 = 36969 * (uint32_t)(rngState & 0xffff) + (uint32_t)((rngState >> 16) & 0xffff);
    return (((uint64_t)r0) << 32) | ((uint64_t)r1);
}

inline
uint32_t randomInt32(uint64_t rngState) {
    return (((uint32_t)(rngState >> 16)) & 0xffff0000) | (((uint32_t)rngState) & 0xffff);
}

inline
double int32ToDouble(uint32_t x) {
    return x * 2.3283064365386962890625e-10;
}

inline
uint32_t doubleToInt32(double x) {
    return (uint32_t)round(x * (double)0x100000000LL);
}

void test(uint64_t rngState) {
    rngState = nextState(rngState);
    printf("Next state: %016llx\n", rngState);
    uint32_t x = randomInt32(rngState);
    printf("Random u32: %08x\n", x);
    double d = int32ToDouble(x);
    printf("Random double: %.17g\n", d);
    uint32_t xx = doubleToInt32(d);
    if (x != xx) {
        printf("Double converted back to u32: %08x\n", xx);
    }
}

int testState(uint64_t rngState, uint32_t *knownInts, int knownIntCount, int maxIterApart) {
    for (; maxIterApart; maxIterApart--) {
        rngState = nextState(rngState);
        if (randomInt32(rngState) == *knownInts) {
            knownInts++;
            if (!--knownIntCount) { return 1; }
        }
    }
    return 0;
}

void crack(double firstDouble, double knownDoubles[], int knownDoubleCount, int maxIterApart) {
    int found = 0;
    uint32_t firstInt = doubleToInt32(firstDouble);
    uint64_t knownMask = (((uint64_t)(firstInt >> 16)) << 32) | ((uint64_t)(firstInt & 0xffff));

    uint32_t knownInts[knownDoubleCount];
    for (int i = 0; i < knownDoubleCount; i++) {
        knownInts[i] = doubleToInt32(knownDoubles[i]);
    }

    for (uint64_t i = 0; i < 0x100000000LL; i++) {
        uint64_t rngState = knownMask | ((i & 0xffff) << 16) | ((i & 0xffff0000) << 32);
        if (testState(rngState, knownInts, knownDoubleCount, maxIterApart)) {
            printf("Found matching RNG state: %016llx\n", rngState);
            found++;
        }
    }
    if (!found) {
        printf("Could not brute force RNG state\n");
    }
}

void usage() {
    printf("Usage: ./random <\"generate\"|\"test\"> <seed_MSB> <seed_LSB>\n");
    printf("   or: ./random crack <first_double> <second_double> <max_iterations_apart>\n");
}

int main(int argc, const char * argv[]) {
    if (argc < 4) { usage(); return 1; }

    if (!strcmp(argv[1], "crack")) {
        if (argc < 5) { usage(); return 1; }
        double firstDouble;
        assert(sscanf(argv[2], "%lf", &firstDouble) == 1);

        int knownDoubleCount = argc - 4;
        double knownDoubles[knownDoubleCount];
        for (int i = 0; i < knownDoubleCount; i++) {
            assert(sscanf(argv[3 + i], "%lf", knownDoubles + i) == 1);
        }

        int maxIterApart;
        assert(sscanf(argv[argc - 1], "%d", &maxIterApart) == 1);

        crack(firstDouble, knownDoubles, knownDoubleCount, maxIterApart);
        return 0;
    }

    uint32_t msb, lsb;
    assert(sscanf(argv[2], "%x", &msb) == 1);
    assert(sscanf(argv[3], "%x", &lsb) == 1);
    uint64_t rngState = (((uint64_t)msb) << 32) | ((uint64_t)lsb);

    if (!strcmp(argv[1], "test")) {
        test(rngState);
        return 0;
    }

    if (!strcmp(argv[1], "generate")) {
        for (int i = 0; i < 50; i++) {
            rngState = nextState(rngState);
            printf("%lf\n", int32ToDouble(randomInt32(rngState)));
        }
        return 0;
    }

    usage();
    return 1;
}