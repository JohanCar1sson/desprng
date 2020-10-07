#include <stdlib.h> /* rand(), rand48() */
#include <limits.h> /* ULONG_MAX */
#include <unif01.h>
#include <ugfsr.h>
#include <bbattery.h>

/* Subjects some common PRNGs to the TestU01 Small Crush test */

/* Converts lrand48 "long int" return value to "unsigned int" */
unsigned lrand48_wrapper();

/* Some homebrew PRNG */
double some_prng();
/* And its seed */
unsigned nkorkad = 7311U;

int main()
{
    unif01_Gen *gen;

    gen = unif01_CreateExternGenBits("stdlib rand()", rand);
    bbattery_SmallCrush(gen);
    unif01_DeleteExternGenBits(gen);

    gen = unif01_CreateExternGenBits("stdlib lrand48()", lrand48_wrapper);
    bbattery_SmallCrush(gen);
    unif01_DeleteExternGenBits(gen);

    gen = unif01_CreateExternGen01("stdlib drand48()", drand48);
    bbattery_SmallCrush(gen);
    unif01_DeleteExternGen01(gen);

    gen = unif01_CreateExternGen01("Some homebrew PRNG", some_prng);
    bbattery_SmallCrush(gen);
    unif01_DeleteExternGen01(gen);

    gen = ugfsr_CreateMT19937_98(ULONG_MAX >> 1);
    bbattery_SmallCrush(gen);
    /* bbattery_Crush(gen); */
    ugfsr_DeleteGen(gen);

    return 0;
}

unsigned lrand48_wrapper()
{
    return (unsigned)lrand48();
}

double some_prng()
{
    double x;
    do
    {
        nkorkad = 1664525U * nkorkad + 1013904223U;
        x = (double)nkorkad / 037777777777U;
    } while (x >= 1.0); /* Make sure x is in the interval [0, 1) */

    return x;
}
