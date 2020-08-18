#include <assert.h>
#include <unif01.h>
#include <bbattery.h>
#include <sys/random.h>
#include <stdio.h>

#include "desprng.h"

/* Subject an alternating pair of DES PRNGs to the TestU01 Crush test.
 * With unmodified DES an odd-even particle pair (with particle numbers
 * that differ only in the least-significant bit) generate identical PRN
 * sequences. This test demonstrates that the modified DES PRNG algorithm
 * does not have this weakness. The output should be similar to this:
 *
 * ========= Summary results of Crush =========
 *
 * Version:          TestU01 1.2.3
 * Generator:        Pair of DES PRNGs
 * Number of statistics:  144
 * Total CPU time:   02:46:30.17
 *
 * All tests were passed
 */

unsigned desprngs();
desprng_type despairing[2];
unsigned long icount = 0UL, iprn64;

int main()
{
    unsigned long nident[2];
    unif01_Gen *gen;

    /* Get a proper (not pseudo) 7-byte random number from the /dev/random device */
    assert(7 == getrandom(nident, 7, GRND_RANDOM));
    nident[1] = nident[0] ^ 1UL;
    printf("%016lX\n%016lX\n", nident[0], nident[1]); /* return 0; */

    /* Initialize the identifier nident and a pair of DES PRNGs */
    assert(!create_identifier(nident));
    assert(!create_identifier(nident + 1));
    initialize_prng(despairing, nident[0]);
    initialize_prng(despairing + 1, nident[1]);

    gen = unif01_CreateExternGenBits("Pair of DES PRNGs", desprngs);
    /* bbattery_SmallCrush(gen); */
    bbattery_Crush(gen);
    unif01_DeleteExternGenBits(gen);

    return 0;
}

unsigned desprngs()
{
    /* Convert single 8-byte pseudo-random number into an array of two 4-byte ones */
    unsigned *iprn32 = (unsigned *)&iprn64;

    if (icount++ % 2 == 0) /* For even icount, create new 8-byte pseudo-random number... */
    {
        /* Alternate between despairing[0] and despairing[1] */
        make_prn(despairing + (icount % 4 == 3), icount / 4, &iprn64);
        return iprn32[0]; /* and return one half of it */
    }
    else /* For odd icount... */
        return iprn32[1]; /* return the half left over from the last call */
}
