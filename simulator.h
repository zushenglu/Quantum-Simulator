#ifndef Qsimulator
#define Qsimulator

#include <complex.h>

struct struct_circuit;

float complex* MX_MAP(float complex * vector, int vec_dim, float complex ** mx, int mx_dim);

void simulate(struct struct_circuit *circuit);

#endif