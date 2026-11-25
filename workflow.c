#include "workflow.h"
#include "qubit.h"
#include "simulator.h"
#include "convertor.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

static int run_dense(Circuit *circuit, void (*simulator)(Circuit *)){
    /* Dense simulators store the number of amplitudes in a signed int. */
    if (circuit->size >= (int)(sizeof(int) * CHAR_BIT - 1)){
        fprintf(stderr, "Too many qubits for the dense simulator's integer indices.\n");
        return 1;
    }

#ifdef _SC_PHYS_PAGES
    /* simulate2 always allocates one matrix. Per-gate simulators need two for CX. */
    int matrices = 1;
    if (simulator == simulate || simulator == simulate_parallel){
        matrices = 0;
        for (int i = 0; i < circuit->size && matrices < 2; i++){
            for (const Operation *op = circuit->Q[i]->next; op; op = op->next){
                matrices = op->impacted_qbts_num > 1 ? 2 : 1;
                if (matrices == 2){
                    break;
                }
            }
        }
    }
    double amplitudes = ldexp(1.0, circuit->size);
    double matrix_storage = matrices * amplitudes * sizeof(float complex);
    double result_storage = sizeof(ResUnit *) + sizeof(ResUnit) + circuit->size + 1;
    double min_bytes = amplitudes * (sizeof(float complex) + fmax(matrix_storage, result_storage));
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages > 0 && page_size > 0 && min_bytes > (double)pages * page_size){
        fprintf(stderr, "Dense simulation needs at least %.1f GiB for this circuit, "
                "more than the machine's physical memory.\n", min_bytes / 1073741824.0);
        return 1;
    }
#endif

    simulator(circuit);
    return 0;
}

static const struct {
    const char *name;
    void (*simulator)(Circuit *);
} workflows[] = {
    {"Dense simulator", simulate},
    {"Dense simulator 2 (per layer)", simulate2},
    {"Dense simulator (parallel)",    simulate_parallel},
};

size_t workflow_count(void){
    return sizeof(workflows) / sizeof(workflows[0]);
}

const char *workflow_name(size_t index){
    return index < workflow_count() ? workflows[index].name : NULL;
}

int workflow_run(size_t index, Circuit *circuit){
    if (index >= workflow_count() || circuit == NULL || circuit->size <= 0){
        fprintf(stderr, "Select a valid workflow and circuit.\n");
        return 1;
    }
    return run_dense(circuit, workflows[index].simulator);
}
