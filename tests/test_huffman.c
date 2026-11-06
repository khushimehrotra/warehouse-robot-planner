#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "huffman.h"

int main(void) {
    Huff h;
    ASSERT_EQ(huff_build(&h, (const unsigned char *)"", 0), -1);   /* empty input */

    const unsigned char *s = (const unsigned char *)"aaaabbc";
    ASSERT_EQ(huff_build(&h, s, 7), 0);
    ASSERT_EQ(huff_bits(&h, s, 7), 10);                /* hand-computed optimum */
    ASSERT_EQ(h.len['a'], 1);

    const unsigned char *one = (const unsigned char *)"aaaa";     /* single symbol */
    ASSERT_EQ(huff_build(&h, one, 4), 0);
    ASSERT_EQ(huff_bits(&h, one, 4), 4);

    srand(11);
    for (int t = 0; t < 300; t++) {
        size_t n = 1 + (size_t)(rand() % 500);
        int alphabet = 1 + rand() % 40;
        unsigned char *data = malloc(n);
        int cnt[256] = {0}, distinct = 0;
        for (size_t i = 0; i < n; i++) {               /* skewed: mostly a, b, c */
            int r = (rand() % 4) ? rand() % 3 : rand() % alphabet;
            data[i] = (unsigned char)('a' + r);
            if (cnt[data[i]]++ == 0) distinct++;
        }
        ASSERT_EQ(huff_build(&h, data, n), 0);
        size_t bits = huff_bits(&h, data, n);
        unsigned char *enc = malloc(bits / 8 + 2);
        unsigned char *dec = malloc(n);

        ASSERT_EQ(huff_encode(&h, data, n, enc), 0);
        ASSERT_EQ(huff_decode(&h, enc, bits, dec, n), 0);
        ASSERT_EQ(memcmp(data, dec, n), 0);            /* round trip must be exact */

        int fixed = 0;                                 /* a plain fixed-length code */
        while ((1 << fixed) < distinct) fixed++;
        if (fixed == 0) fixed = 1;
        ASSERT_EQ(bits <= n * (size_t)fixed, 1);       /* Huffman is never worse */
        free(data); free(enc); free(dec);
    }
    TEST_SUMMARY();
}