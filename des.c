/* The functions below are modified versions of those in d3des.c
 * The most significant change was getting rid of all global variables
 * (to make the code thread safe). K&R was changed to ANSI C89, etc.
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

#include "desprng.h"

/* Signatures for the modified d3des functions that are internal (hence, static) to libdesprng.a */
#pragma acc routine(_usekey) seq
static void _usekey(desprng_individual_t *thread_data, unsigned long *from);
#pragma acc routine(_cookey) seq
static void _cookey(desprng_individual_t *thread_data, unsigned long *raw1);
#pragma acc routine(_scrunch) seq
static void _scrunch(unsigned char *outof, unsigned long *into);
#pragma acc routine(_unscrun) seq
static void _unscrun(unsigned long *outof, unsigned char *into);
#pragma acc routine(_desfunc) seq
static void _desfunc(desprng_common_t *process_data, unsigned long *block, unsigned long *keys);

#pragma acc routine seq
void _deskey(desprng_common_t *process_data, desprng_individual_t *thread_data, unsigned char *key) /* Thanks to James Gillogly & Phil Karn! */
{
    int i, j, l, m, n;
    unsigned char pc1m[56], pcr[56];
    unsigned long kn[32];

    for (j = 0; j < 56; j++)
    {
        l = process_data->pc1[j];
        m = l & 07;
        pc1m[j] = (key[l >> 3] & process_data->bytebit[m]) ? 1 : 0;
    }
    for (i = 0; i < 16; i++)
    {
        m = i << 1;
        n = m + 1;
        kn[m] = kn[n] = 0L;
        for (j = 0; j < 28; j++)
        {
            l = j + process_data->totrot[i];
            if (l < 28) pcr[j] = pc1m[l];
            else pcr[j] = pc1m[l - 28];
        }
        for (j = 28; j < 56; j++)
        {
            l = j + process_data->totrot[i];
            if (l < 56) pcr[j] = pc1m[l];
            else pcr[j] = pc1m[l - 28];
        }
        for (j = 0; j < 24; j++)
        {
            if (pcr[process_data->pc2[j]]) kn[m] |= process_data->bigbyte[j];
            if (pcr[process_data->pc2[j + 24]]) kn[n] |= process_data->bigbyte[j];
        }
    }
    _cookey(thread_data, kn);

    return;
}

static void _cookey(desprng_individual_t *thread_data, unsigned long *raw1)
{
    unsigned long *cook, *raw0;
    unsigned long dough[32];
    int i;

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
    _usekey(thread_data, dough);

    return;
}

static void _usekey(desprng_individual_t *thread_data, unsigned long *from)
{
    unsigned long *to, *endp;

    to = thread_data->KnL; endp = thread_data->KnL + 32;
    while (to < endp) *to++ = *from++;

    return;
}

#pragma acc routine seq
void _des(desprng_common_t *process_data, desprng_individual_t *thread_data, unsigned char *inblock, unsigned char *outblock)
{
    unsigned long work[2];

    _scrunch(inblock, work);
    _desfunc(process_data, work, thread_data->KnL);
    _unscrun(work, outblock);

    return;
}

static void _scrunch(unsigned char *outof, unsigned long *into)
{
    *into    = (*outof++ & 0xffL) << 24;
    *into   |= (*outof++ & 0xffL) << 16;
    *into   |= (*outof++ & 0xffL) << 8;
    *into++ |= (*outof++ & 0xffL);
    *into    = (*outof++ & 0xffL) << 24;
    *into   |= (*outof++ & 0xffL) << 16;
    *into   |= (*outof++ & 0xffL) << 8;
    *into   |= (*outof   & 0xffL);

    return;
}

static void _unscrun(unsigned long *outof, unsigned char *into)
{
    *into++ = (*outof >> 24) & 0xffL;
    *into++ = (*outof >> 16) & 0xffL;
    *into++ = (*outof >>  8) & 0xffL;
    *into++ =  *outof++      & 0xffL;
    *into++ = (*outof >> 24) & 0xffL;
    *into++ = (*outof >> 16) & 0xffL;
    *into++ = (*outof >>  8) & 0xffL;
    *into   =  *outof        & 0xffL;

    return;
}

static void _desfunc(desprng_common_t *process_data, unsigned long *block, unsigned long *keys)
{
    unsigned long fval, work, right, leftt;
    int round;
    
    leftt = block[0];
    right = block[1];
    work = ((leftt >> 4) ^ right) & 0x0f0f0f0fL;
    right ^= work;
    leftt ^= (work << 4);
    work = ((leftt >> 16) ^ right) & 0x0000ffffL;
    right ^= work;
    leftt ^= (work << 16);
    work = ((right >> 2) ^ leftt) & 0x33333333L;
    leftt ^= work;
    right ^= (work << 2);
    work = ((right >> 8) ^ leftt) & 0x00ff00ffL;
    leftt ^= work;
    right ^= (work << 8);
    right = ((right << 1) | ((right >> 31) & 1L)) & 0xffffffffL;
    work = (leftt ^ right) & 0xaaaaaaaaL;
    leftt ^= work;
    right ^= work;
    leftt = ((leftt << 1) | ((leftt >> 31) & 1L)) & 0xffffffffL;
    
    for (round = 0; round < 8; round++)
    {
        work  = (right << 28) | (right >> 4);
        work ^= *keys++;
        fval  = process_data->SP[6][ work        & 0x3fL];
        fval |= process_data->SP[4][(work >>  8) & 0x3fL];
        fval |= process_data->SP[2][(work >> 16) & 0x3fL];
        fval |= process_data->SP[0][(work >> 24) & 0x3fL];
        work  = right ^ *keys++;
        fval |= process_data->SP[7][ work        & 0x3fL];
        fval |= process_data->SP[5][(work >>  8) & 0x3fL];
        fval |= process_data->SP[3][(work >> 16) & 0x3fL];
        fval |= process_data->SP[1][(work >> 24) & 0x3fL];
        leftt ^= fval;
        work  = (leftt << 28) | (leftt >> 4);
        work ^= *keys++;
        fval  = process_data->SP[6][ work        & 0x3fL];
        fval |= process_data->SP[4][(work >>  8) & 0x3fL];
        fval |= process_data->SP[2][(work >> 16) & 0x3fL];
        fval |= process_data->SP[0][(work >> 24) & 0x3fL];
        work  = leftt ^ *keys++;
        fval |= process_data->SP[7][ work        & 0x3fL];
        fval |= process_data->SP[5][(work >>  8) & 0x3fL];
        fval |= process_data->SP[3][(work >> 16) & 0x3fL];
        fval |= process_data->SP[1][(work >> 24) & 0x3fL];
        right ^= fval;
    }
    right = (right << 31) | (right >> 1);
    work = (leftt ^ right) & 0xaaaaaaaaL;
    leftt ^= work;
    right ^= work;
    leftt = (leftt << 31) | (leftt >> 1);
    work = ((leftt >> 8)  ^ right) & 0x00ff00ffL;
    right ^= work;
    leftt ^= (work << 8);
    work = ((leftt >> 2)  ^ right) & 0x33333333L;
    right ^= work;
    leftt ^= (work << 2);
    work = ((right >> 16) ^ leftt) & 0x0000ffffL;
    leftt ^= work;
    right ^= (work << 16);
    work = ((right >> 4)  ^ leftt) & 0x0f0f0f0fL;
    leftt ^= work;
    right ^= (work << 4);
    *block++ = right;
    *block = leftt;

    return;
}
