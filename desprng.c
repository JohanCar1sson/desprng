#include <limits.h>

/* Takes the 56 least significant bits of an unsigned long and splits them into
   8 groups of 7 bits each. Each group becomes the 7 most signficant bits of a
   byte (with its least significant bit is set to zero). The result is an
   identifier in the form of a unique DES key. The implementation should work
   for either endianness, but has only been tested on little-endian systems.

   Copyright (C) 2020 by Johan Carlsson and RadiaSoft LLC
*/

int create_identifier(unsigned long *nident)
{
    unsigned long ntmp;
    unsigned char i;

    /* Make sure *nident < 2**56 to guarantee the uniqueness of the key,
       i.e. to make it an identifier */
    if (*nident >> 56) return -1;

    ntmp = *nident << 1;
    *nident = 0UL;
    /* Build the identifier byte by byte, with descending significance */
    for (i = 0; i < 8; i++)
    {
        /* Make the next group of 7 bits the most significant bits of ntmp */
        ntmp <<= 7;
        /* Add the group as a byte to the identifier
           (with a padded zero for the least significant bit) */
        *nident += (ntmp >> 57) << ((7 - i) * 8 + 1);
    }
    return 0;
}


/* Hopefully self explanatory? */

int check_type_sizes()
{
    if (!(ULONG_MAX == 18446744073709551615UL)) /* Verify that unsigned long is 8 bytes */
        return -3;
    if (!(USHRT_MAX == 65535)) /* Verify that unsigned short is 2 bytes */
        return -2;
    if (!(CHAR_BIT == 8)) /* Verify that a char is 8 bits */
        return -1;

    return 0;
}
