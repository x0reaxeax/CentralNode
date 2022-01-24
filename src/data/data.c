/* minimal dependency data operations */
#include "data.h"

#include <stdio.h>
#include <limits.h>

unsigned char x_ctoi(char c) { return c - 48; }

uint64_t rtonm(uint64_t number, uint64_t multiple) {
    if (number == 0) {
        return multiple;
    }

    if (multiple == 0) {
        return number;
    }

    unsigned long long remainder = number % multiple;
    if (remainder == 0) {
        return number;
    }

    return (number + multiple) - remainder;
}

unsigned int x_numdigits (uint64_t  n) {
    //if (n < 0) n = (n == INT_MIN) ? ULLONG_MAX : - n;
    
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    if (n < 10000000000) return 10;
    if (n < 100000000000) return 11;
    if (n < 1000000000000) return 12;
    if (n < 10000000000000) return 13;
    if (n < 100000000000000) return 14;
    if (n < 1000000000000000) return 15;
    if (n < 10000000000000000) return 16;
    if (n < 100000000000000000) return 17;
    if (n < 1000000000000000000) return 18;
    if (n < 10000000000000000000ULL) return 19;
#ifdef _X___SIZEOF_INT128__
    if (n < (unsigned __int128) 100000000000000000000) return 20;
    if (n < (unsigned __int128) 1000000000000000000000U) return 21;
    if (n < (unsigned __int128) 10000000000000000000000U) return 22;
    if (n < (unsigned __int128) 100000000000000000000000U) return 23;
    if (n < (unsigned __int128) 1000000000000000000000000U) return 24;
    if (n < (unsigned __int128) 10000000000000000000000000U) return 25;
    if (n < (unsigned __int128) 100000000000000000000000000U) return 26;
    if (n < (unsigned __int128) 1000000000000000000000000000U) return 27;
    if (n < (unsigned __int128) 10000000000000000000000000000U) return 28;
    if (n < (unsigned __int128) 100000000000000000000000000000U) return 29;
    if (n < (unsigned __int128) 1000000000000000000000000000000U) return 30;
    if (n < (unsigned __int128) 10000000000000000000000000000000U) return 31;
    if (n < (unsigned __int128) 100000000000000000000000000000000U) return 32;
    if (n < (unsigned __int128) 1000000000000000000000000000000000U) return 33;
    if (n < (__uint128_t) 10000000000000000000000000000000000U) return 34;
    if (n < (__uint128_t) 100000000000000000000000000000000000U) return 35;
    if (n < (__uint128_t) 1000000000000000000000000000000000000U) return 36;
    if (n < (__uint128_t) 10000000000000000000000000000000000000U) return 37;
    if (n < (__uint128_t) 100000000000000000000000000000000000000U) return 38;

    return 39;

#endif
    /*      2147483647 is 2^31-1 - add more ifs as needed
       and adjust this final return as well. */
    return 20;
}