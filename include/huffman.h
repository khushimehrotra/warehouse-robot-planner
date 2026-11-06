#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <stddef.h>

typedef struct { int left, right, sym, freq; } HNode;   /* leaf: left == right == -1 */

typedef struct {
    HNode nodes[511];               /* at most 256 leaves + 255 internal nodes */
    int nnodes, root;
    unsigned long long code[256];   /* code bits, written in the low 'len' bits */
    int len[256];                   /* code length in bits; 0 = symbol not present */
} Huff;

/* Build the tree and code table from the data.
   Return: 0 ok, -1 empty input, -2 input too long, -3 out of memory. */
int huff_build(Huff *h, const unsigned char *data, size_t n);

/* Total bits needed to encode data with this table. */
size_t huff_bits(const Huff *h, const unsigned char *data, size_t n);

/* out must have (huff_bits + 7) / 8 bytes. Return 0 ok, -1 symbol not in table. */
int huff_encode(const Huff *h, const unsigned char *data, size_t n, unsigned char *out);

/* Decode exactly n symbols from 'bits' bits. Return 0 ok, -4 corrupt data. */
int huff_decode(const Huff *h, const unsigned char *in, size_t bits,
                unsigned char *out, size_t n);

#endif