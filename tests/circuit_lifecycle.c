#include <assert.h>
#include <complex.h>
#include <stdio.h>

#include "circuit.h"
#include "qubit.h"

int main(void){
    FREE_CIRCUIT(NULL);
    assert(INIT_CIRCUIT(0) == NULL);

    for (int iteration = 0; iteration < 100; iteration++){
        Circuit *circuit = INIT_CIRCUIT(3);
        assert(circuit != NULL);

        RZ(circuit, 0, 0.5f);
        Hadamard(circuit, 1);
        CX(circuit, 0, 2);
        PauliX(circuit, 2);

        assert(circuit->Q[0]->head->impacted_qbts[0] == 0);
        assert(circuit->Q[0]->head->next->gate == circuit->Q[2]->head->gate);

        /* Simulate the cursor reaching the end; cleanup still starts at head. */
        for (int i = 0; i < circuit->size; i++){
            circuit->Q[i]->next = NULL;
        }
        FREE_CIRCUIT(circuit);
    }

    puts("Circuit lifecycle test passed");
    return 0;
}
