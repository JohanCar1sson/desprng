#include <limits.h>

#include "desprng.h"

/* These global variables in d3des are only read, never written to, so are left as is */
static unsigned char pc1[56] = {
	56, 48, 40, 32, 24, 16,  8,	 0, 57, 49, 41, 33, 25, 17,
	 9,  1, 58, 50, 42, 34, 26,	18, 10,  2, 59, 51, 43, 35,
	62, 54, 46, 38, 30, 22, 14,	 6, 61, 53, 45, 37, 29, 21,
	13,  5, 60, 52, 44, 36, 28,	20, 12,  4, 27, 19, 11,  3 };

static unsigned char pc2[48] = {
	13, 16, 10, 23,  0,  4,	 2, 27, 14,  5, 20,  9,
	22, 18, 11,  3, 25,  7,	15,  6, 26, 19, 12,  1,
	40, 51, 30, 36, 46, 54,	29, 39, 50, 44, 32, 47,
	43, 48, 38, 55, 33, 52,	45, 41, 49, 35, 28, 31 };

static unsigned char totrot[16] = {
	1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28 };

static unsigned short bytebit[8]	= {
	0200, 0100, 040, 020, 010, 04, 02, 01 };

static unsigned long bigbyte[24] = {
	0x800000L,	0x400000L,	0x200000L, 	0x100000L,
	0x80000L,	0x40000L,	0x20000L,	0x10000L,
	0x8000L,	0x4000L,	0x2000L,	0x1000L,
	0x800L,		0x400L,		0x200L,		0x100L,
	0x80L,		0x40L,		0x20L,		0x10L,
	0x8L,		0x4L,		0x2L,		0x1L	};

void _deskey(desprng_type *despairing, unsigned char *key);
void _usekey(desprng_type *despairing, register unsigned long *from);
static void _cookey(desprng_type *despairing, register unsigned long *raw1);

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


/* Foo.

   Copyright (C) 2020 by Johan Carlsson and RadiaSoft LLC
*/



/* Hopefully self explanatory... */

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

/* The functions below are modified versions of those in d3des.c
 * The most significant change is getting rid of all global variables
 * to make the code thread safe. K&R was changed to ANSI C89, etc.
 *
 * Johan Carlsson, August 14, 2020
 */

/* D3DES (V5.09) - 
 *
 * A portable, public domain, version of the Data Encryption Standard.
 *
 * Written with Symantec's THINK (Lightspeed) C by Richard Outerbridge.
 * Thanks to: Dan Hoey for his excellent Initial and Inverse permutation
 * code;  Jim Gillogly & Phil Karn for the DES key schedule code; Dennis
 * Ferguson, Eric Young and Dana How for comparing notes; and Ray Lau,
 * for humouring me on. 
 *
 * Copyright (c) 1988,1989,1990,1991,1992 by Richard Outerbridge.
 * (GEnie : OUTER; CIS : [71755,204]) Graven Imagery, 1992.
 */


/* Copyright (c) 1988,1989,1990,1991,1992 by Richard Outerbridge.
 * Copyright (C) 2020 by Johan Carlsson and RadiaSoft LLC
 */

int initialize_prng(desprng_type *despairing, unsigned long nident)
{
    unsigned i;

    despairing->nident = nident;
    for (i = 0; i < 32; i++)
        despairing->Kn3[i] = despairing->KnR[i] = despairing->KnL[i] = 0UL;

    _deskey(despairing, (unsigned char *)&nident);

    return 0;
}

void _deskey(desprng_type *despairing, unsigned char *key) /* Thanks to James Gillogly & Phil Karn! */
{
    register int i, j, l, m, n;
    unsigned char pc1m[56], pcr[56];
    unsigned long kn[32];

    for (j = 0; j < 56; j++)
    {
        l = pc1[j];
        m = l & 07;
        pc1m[j] = (key[l >> 3] & bytebit[m]) ? 1 : 0;
    }
    for (i = 0; i < 16; i++)
    {
        /* if (edf == DE1) m = (15 - i) << 1; */ /* Decryption, not relevant for PRNG */
        /* else */ m = i << 1;
        n = m + 1;
        kn[m] = kn[n] = 0L;
        for (j = 0; j < 28; j++)
        {
            l = j + totrot[i];
            if (l < 28) pcr[j] = pc1m[l];
            else pcr[j] = pc1m[l - 28];
        }
        for (j = 28; j < 56; j++)
        {
            l = j + totrot[i];
            if (l < 56) pcr[j] = pc1m[l];
            else pcr[j] = pc1m[l - 28];
        }
        for (j = 0; j < 24; j++)
        {
            if (pcr[pc2[j]]) kn[m] |= bigbyte[j];
            if (pcr[pc2[j + 24]]) kn[n] |= bigbyte[j];
        }
    }
    _cookey(despairing, kn);

    return;
}


/* Copyright (c) 1988,1989,1990,1991,1992 by Richard Outerbridge.
 * Copyright (C) 2020 by Johan Carlsson and RadiaSoft LLC
 */

static void _cookey(desprng_type *despairing, register unsigned long *raw1)
{
    register unsigned long *cook, *raw0;
    unsigned long dough[32];
    register int i;

    cook = dough;
    for (i = 0; i < 16; i++, raw1++)
    {
        raw0 = raw1++;
        *cook    = (*raw0 & 0x00fc0000L) << 6;
        *cook   |= (*raw0 & 0x00000fc0L) << 10;
        *cook   |= (*raw1 & 0x00fc0000L) >> 10;
        *cook++ |= (*raw1 & 0x00000fc0L) >> 6;
        *cook    = (*raw0 & 0x0003f000L) << 12;
        *cook   |= (*raw0 & 0x0000003fL) << 16;
        *cook   |= (*raw1 & 0x0003f000L) >> 4;
        *cook++ |= (*raw1 & 0x0000003fL);
    }
    _usekey(despairing, dough);

    return;
}

#if 0
/* Copyright (c) 1988,1989,1990,1991,1992 by Richard Outerbridge.
 * Copyright (C) 2020 by Johan Carlsson and RadiaSoft LLC
 */

cpkey(into)
register unsigned long *into;
{
    register unsigned long *from, *endp;

    from = KnL, endp = &KnL[32];
    while(from < endp) *into++ = *from++;
    return;
    }
#endif


/* Copyright (c) 1988,1989,1990,1991,1992 by Richard Outerbridge.
 * Copyright (C) 2020 by Johan Carlsson and RadiaSoft LLC
 */

void _usekey(desprng_type *despairing, register unsigned long *from)
{
    register unsigned long *to, *endp;

    to = despairing->KnL; endp = despairing->KnL + 32;
    while (to < endp) *to++ = *from++;

    return;
}

#if 0
/* Copyright (c) 1988,1989,1990,1991,1992 by Richard Outerbridge.
 * Copyright (C) 2020 by Johan Carlsson and RadiaSoft LLC
 */

des(inblock, outblock)
unsigned char *inblock, *outblock;
{
    unsigned long work[2];

    scrunch(inblock, work);
    desfunc(work, KnL);
    unscrun(work, outblock);
    return;
    }
#endif
