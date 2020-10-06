/* This is the header file for the libdesprng library.
 * The DES Pseudo-Random Number Generator (PRNG) can be executed at the thread
 * level, making it suitable for GPU computing with OpenACC, and similar.
 * It is based on the Data Encryption Standard (DES) block cipher.
 * Compared to the Mersenne Twister (MT), it only has 7 bytes of state
 * (vs. ~2,500 bytes), uses 80% more computation, and passes all 144 tests of
 * the Crush suite (vs. 142 tests passed by MT), as well as all 126 tests of
 * the DIEHARD suite and the 4 tests of FIPS-140-2. The DES PRNG allows up to
 * 2**56 threads to produce uncorrelated PRN sequences with a period of 2**64.
 *
 * See the source file for copyright on the individual functions.
 *
 * Author: Johan Carlsson
*/

/* Data structure that each thread needs a private copy of */
typedef struct desprng_thread_variables
{
    /* This is the unique PRNG identifier */
    unsigned long nident;
    /* These three arrays are expanded versions of the indentifier.
       They are not striclty necessary, but allow for faster PRN generation. */
    unsigned long KnL[32];
    unsigned long KnR[32];
    unsigned long Kn3[32];
}
desprng_individual_t;

/* Read-only data structure accessed by all threads */
typedef struct desprng_process_variables
{
    unsigned char pc1[56];
    unsigned char pc2[48];
    unsigned char totrot[16];
    unsigned short bytebit[8];
    unsigned long bigbyte[24];
    unsigned long SP[8][64];
}
desprng_common_t;

/* Signatures for the user interface */

#pragma acc routine(initialize_common) seq
int initialize_common(desprng_common_t *process_data);

#pragma acc routine(create_identifier) seq
int create_identifier(unsigned long *nident);

#pragma acc routine(initialize_individual) seq
int initialize_individual(desprng_common_t *process_data, desprng_individual_t *thread_data, unsigned long nident);

#pragma acc routine(make_prn) seq
int make_prn(desprng_common_t *process_data, desprng_individual_t *thread_data, unsigned long icount, unsigned long *iprn);

#pragma acc routine(get_uniform_prn) seq
double get_uniform_prn(desprng_common_t *process_data, desprng_individual_t *thread_data, unsigned long icount, unsigned long *iprn);

int check_type_sizes();
