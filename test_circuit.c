#include <stddef.h>
#include "Q-sim.h"

Circuit *build_large_circuit(int size)
{
    Circuit *circuit = INIT_CIRCUIT(size);
    if (circuit == NULL) {
        return NULL;
    }

    /* Five groups of eight gates, wrapping wire indices at the circuit size. */
    for (int i = 0; i < 8; i++) {
        Hadamard(circuit, (2 * i) % size);
    }
    for (int i = 0; i < 8; i++) {
        CX(circuit, (2 * i) % size, (2 * i + 1) % size);
    }
    for (int i = 0; i < 8; i++) {
        RZ(circuit, (2 * i + 1) % size, 0.25f * (i % 4 + 1));
    }
    for (int i = 0; i < 8; i++) {
        CX(circuit, (2 * i + 1) % size, (2 * i + 2) % size);
    }
    for (int i = 0; i < 8; i++) {
        Hadamard(circuit, (2 * i) % size);
    }

    return circuit;
}


Circuit *build_small_circuit(int size)
{
    Circuit *circuit = INIT_CIRCUIT(size);
    if (circuit == NULL) {
        return NULL;
    }

    /* Put the even-numbered qubits in superposition. */
    Hadamard(circuit, 0);
    Hadamard(circuit, 2);
    Hadamard(circuit, 4);
    Hadamard(circuit, 6);

    /* Entangle each adjacent pair. */
    CX(circuit, 0, 1);
    CX(circuit, 2, 3);
    CX(circuit, 4, 5);
    CX(circuit, 6, 7);

    /* Apply phase rotations in radians. */
    RZ(circuit, 1, 0.25f);
    RZ(circuit, 3, 0.50f);
    RZ(circuit, 5, 0.75f);
    RZ(circuit, 7, 1.00f);

    /* Connect the pairs in a ring. */
    CX(circuit, 1, 2);
    CX(circuit, 3, 4);
    CX(circuit, 5, 6);
    CX(circuit, 7, 0);

    Hadamard(circuit, 0);
    Hadamard(circuit, 2);
    Hadamard(circuit, 4);
    Hadamard(circuit, 6);

    return circuit;
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <workflow index> <qubits>\n", argv[0]);
        return 1;
    }

    char *end;
    long algo = strtol(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0' || algo < 0 || (size_t)algo >= workflow_count()) {
        fprintf(stderr, "Invalid workflow index: %s\n", argv[1]);
        return 1;
    }

    long qubits = strtol(argv[2], &end, 10);
    if (*end != '\0' || qubits < 8 || qubits > 16) {
        fprintf(stderr, "Supported qubit sizes are 8 through 16.\n");
        return 1;
    }

    int size = (int)qubits;
    Circuit *circuit = size == 8 ? build_small_circuit(size) : build_large_circuit(size);
    if (circuit == NULL) {
        return 1;
    }

    PRINT_CIRCUIT(circuit, size);
    int status = workflow_run((size_t)algo, circuit);
    FREE_CIRCUIT(circuit);
    return status;
}
