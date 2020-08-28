#include <assert.h>
#include <unif01.h>
#include <bbattery.h>
#include <sys/random.h>

#include "desprng.h"

/* Subject a single DES PRNG to the TestU01 Small Crush test.
   It should pass all the tests! */

unsigned desprng();
desprng_common_t process_data;
desprng_individual_t thread_data;
unsigned long icount = 0UL, iprn64;

int main()
{
    unsigned long nident;
    unif01_Gen *gen;

    /* Get a proper (not pseudo) 7-byte random number from the /dev/random device */
    assert(7 == getrandom(&nident, 7, GRND_RANDOM));
    /* On big-endian computers, the next line would be needed */
    /* nident >>= 8; */

    /* Initialize the identifier nident and a DES PRNG */
    assert(!create_identifier(&nident));
    initialize_common(&process_data);
    initialize_individual(&process_data, &thread_data, nident);

    gen = unif01_CreateExternGenBits("DES PRNG", desprng);
    bbattery_SmallCrush(gen);
    /* bbattery_Crush(gen); */
    /* bbattery_BigCrush(gen); */
    unif01_DeleteExternGenBits(gen);

    return 0;
}

unsigned desprng()
{
    /* Convert single 8-byte pseudo-random number into an array of two 4-byte ones */
    unsigned *iprn32 = (unsigned *)&iprn64;

    if (icount++ % 2 == 0) /* For even icount, create new 8-byte pseudo-random number... */
    {
        make_prn(&process_data, &thread_data, icount / 2, &iprn64);
        return iprn32[0]; /* and return one half of it */
    }
    else /* For odd icount... */
        return iprn32[1]; /* return the half left over from the last call */
}
