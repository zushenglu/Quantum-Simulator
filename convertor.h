#ifndef Q_conv
#define Q_conv

#include <complex.h>

typedef struct struct_result_unit {
    float prob;
    char* state;
} ResUnit;

ResUnit** to_prob(float complex* arr, int sv_len);
void PRINT_RESULT(ResUnit** result, int res_len);

#endif