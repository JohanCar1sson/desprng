#include <assert.h>
#include <stdio.h>
#include <sys/random.h>

#include "desprng.h"
#include "d3des.h"

int main(int argc, char *argv[])
{
    unsigned long nident, Ntime = 4, itime, icount, iprn;
    unsigned short Ncoll = 4, icoll;
    desprng_common_t process_data;
    desprng_individual_t thread_data;
    FILE *d3desdump, *desprngdump;

    assert(!(Ntime >> 48)); /* Make sure Ntime < 2**48 */

    /* Initialize the identifier nident and the old d3des "library" */

    /* Get a proper (not pseudo) 7-byte random number from the /dev/random device */
    assert(7 == getrandom(&nident, 7, GRND_RANDOM));
    /* On big-endian computers, the next line would be needed */
    /* nident >>= 8; */
    assert(!create_identifier(&nident));

    /* deskey() does not change nident, but stores it as expanded internal (and global!) state */
    deskey((unsigned char *)&nident, 0);

    /* Initialize one of the new DES PRNG */
    initialize_common(&process_data);
    initialize_individual(&process_data, &thread_data, nident);

    if (!(d3desdump = fopen("d3des.out", "w"))) return -1;
    if (!(desprngdump = fopen("desprng.out", "w"))) return -1;

    for (itime = 0UL; itime < Ntime; itime++)
    {
        if (itime)
        {
            fprintf(d3desdump, "\n");
            fprintf(desprngdump, "\n");
        }
        for (icoll = 0; icoll < Ncoll; icoll++)
        {
            /* Make itime the high six bytes of icount, and icoll the low two bytes */
            icount = (itime << 16) + icoll;
            des((unsigned char *)&icount, (unsigned char *)&iprn);
            fprintf(d3desdump, " %016lX", iprn);
            make_prn(&process_data, &thread_data, icount, &iprn);
            fprintf(desprngdump, " %016lX", iprn);
        }
    }
    fprintf(d3desdump, "\n");
    fclose(d3desdump);
    fprintf(desprngdump, "\n");
    fclose(desprngdump);

    return 0;
}
