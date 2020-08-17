#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "desprng.h"

int main(int argc, char *argv[])
{
    unsigned long Npart = 5, Ntime = 4, ipart, itime, icount, iprn, *nident;
    unsigned short Ncoll = 2, icoll;
    desprng_type *despairing;
    double xprn;
    int ierr;

    assert(!(Npart >> 56)); /* Make sure Npart < 2**56 */
    assert(!(Ntime >> 48)); /* Make sure Ntime < 2**48 */
    assert(!(Ncoll >> 16)); /* Make sure Ncoll < 2**16 */

    /* Check that the assumptions the desprng library makes about integer type sizes are valid */
    if (ierr = check_type_sizes())
    {
        fprintf(stderr, "check_type_sizes() returned the error code %d ()", ierr);
        return ierr;
    }
    /* Make some workspace on the stack for the DES PRNGs */
    nident = alloca(8 * Npart);
    despairing = alloca(sizeof(desprng_type) * Npart);

    for (itime = 0UL; itime < Ntime; itime++)
        for (ipart = 0UL; ipart < Npart; ipart++)
        {
            if (!itime)
            {
                /* Create unique DES PRNG identifier using particle number */
                nident[ipart] = ipart;
                create_identifier(nident + ipart);
                /* Initialize one DES PRNG for each particle */
                initialize_prng(despairing + ipart, nident[ipart]);
            }
            for (icoll = 0; icoll < Ncoll; icoll++)
            {
                /* Make itime the high six bytes of icount, and icoll the low two bytes */
                icount = (itime << 16) + icoll;
                /* Create the unsigned long pseudo-random number iprn */
                make_prn(despairing + ipart, icount, &iprn);
                /* Now get PRN in the form of double-precision float xprn, normalized to [0, 1) */
                xprn = get_uniform_prn(despairing + ipart, icount, &iprn);
                /* In a real PIC-MCC code, we'd use the PRNs in a collision model, here we'll just print them */
                printf("itime = %lu, ipart = %lu, icoll = %hu, iprn = 0x%016lX, xprn = %18.16lf\n", itime, ipart, icoll, iprn, xprn);
            }
        }
    return 0;
}
