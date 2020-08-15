/* This is the header file for the libdesprng library.
 * The DES Pseudo-Random Number Generator (PRNG) can be executed at the thread
 * level, making it suitable for GPU computing with OpenACC, and similar.
 * It is based on the Data Encryption Standard (DES) block cipher.
 * Compared to the Mersenne Twister (MT), it only has 7 bytes of state
 * (vs. ~2,500 bytes), uses 80% more computation, and passes all 144 tests of
 * the Crush suite (vs. 142 tests passed by MT). The DES PRNG allows up to
 *  2**56 threads to produce uncorrelated PRN sequences with a period of 2**64.
 *
 * See the source file for copyright on the individual functions.
 *
 * Author: Johan Carlsson
*/

/* Data structure that each thread needs a private copy of */
typedef struct desprng_struct
{
    /* This is the unique PRNG identifier */
    unsigned long nident;
    /* These three arrays are expanded versions of the indentifier.
       They are not striclty necessary, but allow for faster PRN generation. */
    unsigned long KnL[32];
    unsigned long KnR[32];
    unsigned long Kn3[32];
}
desprng_type;

/* Signatures for the user interface */
int create_identifier(unsigned long *nident);
int initialize_prng(desprng_type *despairing, unsigned long nident);
int make_prn(desprng_type *despairing, unsigned long icount, unsigned long *iprn);
double get_uniform_prn(desprng_type *despairing, unsigned long icount, unsigned long *iprn);
int check_type_sizes();
