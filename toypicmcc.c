#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "desprng.h"

int main(int argc, char *argv[])
{
    unsigned long Npart = 5, Ntime = 4, ipart, itime, icount, iprn, *nident;
    unsigned short Ncoll = 2, icoll;
    desprng_common_t *process_data;
    desprng_individual_t *thread_data;
    double xprn, xaverage = 0.0, xvariance = 0.0;
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
/* #ifndef _OPENACC */
    nident = alloca(8 * Npart);
    thread_data = alloca(sizeof(desprng_individual_t) * Npart);
    process_data = alloca(sizeof(desprng_common_t));
/* #endif */
    /* It looks like it's necessary to allocate memory on the host for create() to work on the device? */
    /* #pragma acc data copy(xaverage, xvariance)
    #pragma acc enter data create(nident[:Npart], thread_data[:Npart], process_data[:1])
    #pragma acc enter data create(process_data[0].pc1[:56], process_data[0].pc2[:48], process_data[0].totrot[:16], process_data[0].bytebit[:8], process_data[0].bigbyte[:24]) */
    initialize_common(process_data);

    for (itime = 0UL; itime < Ntime; itime++)
    {
        #pragma acc parallel
        for (ipart = 0UL; ipart < Npart; ipart++)
        {
            if (!itime)
            {
                /* Create unique DES PRNG identifier using particle number */
                nident[ipart] = ipart;
                create_identifier(nident + ipart);
                printf("nident[%lu] = 0x%016lX\n", ipart, nident[ipart]);
                /* Initialize one DES PRNG for each particle */
                initialize_individual(process_data, thread_data + ipart, nident[ipart]);
            }
            #pragma acc loop reduction(+: xaverage, xvariance)
            for (icoll = 0; icoll < Ncoll; icoll++)
            {
                /* Make itime the high six bytes of icount, and icoll the low two bytes */
#ifdef _OPENACC
                icount = itime << 16; icount += icoll; /* Workaround for pgcc compiler bug */
#else
                icount = (itime << 16) + icoll;
#endif
                /* Create the unsigned long pseudo-random number iprn */
                make_prn(process_data, thread_data + ipart, icount, &iprn);
                /* Now get PRN in the form of double-precision float xprn, normalized to [0, 1) */
                xprn = get_uniform_prn(process_data, thread_data + ipart, icount, &iprn);
                xaverage += xprn;
                xvariance += (xprn - 0.5) * (xprn - 0.5);
                /* In a real PIC-MCC code, we'd use the PRNs in a collision model, here we'll just print them */
                printf("itime = %lu, ipart = %lu, icoll = %hu, icount = %lu, iprn = 0x%016lX, xprn = %18.16lf\n", itime, ipart, icoll, icount, iprn, xprn);
            }
        }
        #pragma acc wait
    }
    /* #pragma acc exit data delete(process_data[0].pc1[:56], process_data[0].pc2[:48], process_data[0].totrot[:16], process_data[0].bytebit[:8], process_data[0].bigbyte[:24])
    #pragma acc exit data delete(nident[:Npart], thread_data[:Npart], process_data[:1]) */

    xaverage /= Ntime * Npart * Ncoll;
    xvariance /= Ntime * Npart * Ncoll;
    printf("average = %18.16lf / 2, variance = %18.16lf / 12\n", 2.0 * xaverage, 12.0 * xvariance);

    return 0;
}
