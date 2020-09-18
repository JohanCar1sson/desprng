#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "desprng.h"

int main(int argc, char *argv[])
{
    unsigned long Npart = 400, Ntime = 4, ipart, itime, icount, iprn, *nident;
    unsigned short Ncoll = 2, icoll;
    desprng_common_t *process_data;
    desprng_individual_t *thread_data;
    double xprn, zeta, czeta, zaverage = 0.0, zvariance = 0.0, dt = 1.0e-2, xt, *xi;
    int ierr;
    /* struct coord { double x, xi; } *coords; */
    FILE *xidump;

    assert(!(Npart >> 56)); /* Make sure Npart < 2**56 */
    assert(!(Ntime >> 48)); /* Make sure Ntime < 2**48 */
    assert(!(Ncoll >> 16)); /* Make sure Ncoll < 2**16 */

    /* Check that the assumptions the desprng library makes about integer type sizes are valid */
    if (ierr = check_type_sizes())
    {
        fprintf(stderr, "check_type_sizes() returned the error code %d ()", ierr);
        return ierr;
    }
    czeta = sqrt(12.); /* PRN normalization constant */
    xt = Ntime * dt;

    xidump = fopen("xi.dat", "w");

    /* Make some workspace on the stack for the DES PRNGs */
/* #ifndef _OPENACC */
    nident = alloca(8 * Npart);
    thread_data = alloca(sizeof(desprng_individual_t) * Npart);
    process_data = alloca(sizeof(desprng_common_t));
    xi = alloca(8 * Npart);
    /* coords = alloca(sizeof(struct coord) * Npart); */
/* #endif */
    /* It looks like it's necessary to allocate memory on the host for create() to work on the device? */
    /* #pragma acc data copyin(Ntime, Npart, Ncoll)
    #pragma acc data copy(xaverage, xvariance)
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
                /* printf("nident[%lu] = 0x%016lX\n", ipart, nident[ipart]); */
                /* Initialize one DES PRNG for each particle */
                initialize_individual(process_data, thread_data + ipart, nident[ipart]);
                /* Initialize particle pitch coordinate */
                xi[ipart] = M_SQRT1_2; /* 45 degree pitch angle */
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
                /* make_prn(process_data, thread_data + ipart, icount, &iprn); */
                /* Now get PRN in the form of double-precision float xprn, normalized to [0, 1) */
                xprn = get_uniform_prn(process_data, thread_data + ipart, icount, &iprn);
                /* Print the PRN */
                /* printf("itime = %lu, ipart = %lu, icoll = %hu, icount = %lu, iprn = 0x%016lX, xprn = %18.16lf\n", itime, ipart, icoll, icount, iprn, xprn); */
                /* Make a zero-mean, unit-variance uniform random number */
                zeta = czeta * (xprn - 0.5); 
                /* Collect some statistics for later */
                zaverage += zeta;
                zvariance += zeta * zeta;
                /* Do Monte-Carlo pitch-angle scattering (in Ncoll substeps) */
                xi[ipart] += -2.0 * xi[ipart] * dt / Ncoll + zeta * sqrt(2.0 * (1.0 - xi[ipart] * xi[ipart]) * dt / Ncoll);
            }
        }
        #pragma acc wait
    }
    /* #pragma acc exit data delete(process_data[0].pc1[:56], process_data[0].pc2[:48], process_data[0].totrot[:16], process_data[0].bytebit[:8], process_data[0].bigbyte[:24])
    #pragma acc exit data delete(nident[:Npart], thread_data[:Npart], process_data[:1]) */

    zaverage /= Ntime * Npart * Ncoll;
    zvariance /= Ntime * Npart * Ncoll;
    printf("average = %18.16lf, variance = %18.16lf\n", zaverage, zvariance);

    /* Dump the particle pitches for post processing */
    fwrite(&Npart, 8, 1, xidump);
    fwrite(&xt, 8, 1, xidump);
    fwrite(xi, 8, Npart, xidump);
    fclose(xidump);

    return 0;
}
