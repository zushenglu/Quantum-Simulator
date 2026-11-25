#ifndef Qsimulator
#define Qsimulator

#include <complex.h>

struct struct_circuit;

float complex* MX_MAP(float complex * vector, int vec_dim, float complex ** mx, int mx_dim);
float complex* MX_MAP2(float complex * vector, int vec_dim, int dimension, float complex mx[dimension][dimension]);

void simulate(struct struct_circuit *circuit);
void simulate2(struct struct_circuit *circuit);

#endif
