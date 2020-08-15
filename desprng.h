typedef struct desprng_struct
{
    unsigned long nident;
    unsigned long KnL[32];
    unsigned long KnR[32];
    unsigned long Kn3[32];
} desprng_type;

int create_identifier(unsigned long *nident);
int initialize_prng(desprng_type *despairing, unsigned long nident);
int make_prn(desprng_type *despairing, unsigned long icount, unsigned long *iprn);
int check_type_sizes();
