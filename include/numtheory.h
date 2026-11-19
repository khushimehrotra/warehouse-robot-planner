#ifndef NUMTHEORY_H
#define NUMTHEORY_H

#define CRT_MAX_MOD 2147483647LL

long long gcd_ll(long long a, long long b);            /* >= 0; gcd(0,0) = 0 */

/* Inverse of a modulo m (m >= 1). Return: 0 ok, -1 bad m, -2 no inverse (gcd != 1). */
int mod_inverse(long long a, long long m, long long *inv);

/* Smallest x >= 0 with x = r[i] (mod m[i]) for all i; the full solution is x + k*lcm.
   Moduli need not be coprime. 1 <= m[i] <= CRT_MAX_MOD; any residue sign is fine.
   Return: 0 ok, -1 bad arguments, -2 no solution, -3 lcm too large for 64 bits. */
int crt(const long long *r, const long long *m, int n, long long *x, long long *lcm);

#endif