#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "circuit.h"
#include "gate.h"
#include "qubit.h"
#include "workflow.h"

static CircuitGateEdit edit(CircuitGateKind kind, int wire, int depth,
                           int target, float angle) {
    CircuitGateEdit result = {kind, wire, depth, target, angle};
    return result;
}

static void test_out_of_order_insert_and_edit(void) {
    Circuit *c = INIT_CIRCUIT(2);
    assert(circuit_place_gate(c, edit(CIRCUIT_X, 0, 3, -1, 0)) == NULL);
    assert(circuit_place_gate(c, edit(CIRCUIT_H, 0, 1, -1, 0)) == NULL);
    assert(circuit_place_gate(c, edit(CIRCUIT_RZ, 0, 2, -1, 0.25f)) == NULL);

    const Operation *op = circuit_gate_at(c, 0, 1);
    assert(op != NULL && op->depth == 1);
    op = circuit_gate_at(c, 0, 2);
    assert(op != NULL && op->depth == 2 && op->param_num == 1);
    assert(fabsf(crealf(op->parameters[0]) - 0.25f) < 1e-5f);
    op = circuit_gate_at(c, 0, 3);
    assert(op != NULL && op->depth == 3);

    assert(circuit_update_gate(c, 0, 2,
                               edit(CIRCUIT_RZ, 0, 2, -1, 1.125f)) == NULL);
    op = circuit_gate_at(c, 0, 2);
    assert(op != NULL && fabsf(crealf(op->parameters[0]) - 1.125f) < 1e-5f);
    FREE_CIRCUIT(c);
}

static void test_cx_target_removal_relinks_and_recomputes(void) {
    Circuit *c = INIT_CIRCUIT(3);
    assert(circuit_place_gate(c, edit(CIRCUIT_CX, 0, 1, 2, 0)) == NULL);
    assert(circuit_place_gate(c, edit(CIRCUIT_X, 2, 2, -1, 0)) == NULL);
    assert(circuit_place_gate(c, edit(CIRCUIT_H, 1, 4, -1, 0)) == NULL);
    assert(circuit_remove_gate(c, 2, 1) == NULL); /* Remove via CX target. */

    assert(circuit_gate_at(c, 0, 1) == NULL);
    assert(c->Q[0]->last == NULL && c->Q[0]->depth == 0);
    assert(circuit_gate_at(c, 2, 1) == NULL);
    assert(circuit_gate_at(c, 2, 2) != NULL);
    assert(circuit_gate_at(c, 2, 2)->depth == 2);
    assert(c->Q[2]->last == (Operation *)circuit_gate_at(c, 2, 2));
    assert(c->Q[2]->depth == 2 && c->depth == 4);
    assert(circuit_gate_at(c, 1, 4) != NULL);
    FREE_CIRCUIT(c);
}

static void test_update_relocates_across_wires(void) {
    Circuit *c = INIT_CIRCUIT(3);
    assert(circuit_place_gate(c, edit(CIRCUIT_RZ, 0, 1, -1, 0.75f)) == NULL);
    assert(circuit_place_gate(c, edit(CIRCUIT_X, 1, 2, -1, 0)) == NULL);
    assert(circuit_update_gate(c, 0, 1,
                               edit(CIRCUIT_H, 2, 3, -1, 0)) == NULL);
    assert(circuit_gate_at(c, 0, 1) == NULL);
    assert(circuit_gate_at(c, 1, 2) != NULL);
    assert(circuit_gate_at(c, 2, 3) != NULL);
    assert(strcmp(circuit_gate_at(c, 2, 3)->name, "H") == 0);
    FREE_CIRCUIT(c);
}

static void test_legacy_borrowed_inputs_remain_caller_owned(void) {
    Circuit *c = INIT_CIRCUIT(1);
    Gate *gate = initH();
    float complex parameter = 0.375f;
    char name[] = "borrowed";
    Add_OP(gate, 0, c, &parameter, 1, name);
    assert(circuit_gate_at(c, 0, 1)->gate == gate);
    assert(circuit_gate_at(c, 0, 1)->parameters == &parameter);
    assert(circuit_remove_gate(c, 0, 1) == NULL);
    FREE_CIRCUIT(c);
    /* Removing a node must preserve caller-owned data. */
    assert(fabsf(crealf(parameter) - 0.375f) < 1e-5f);
    FREE_GATE(gate);
}

static int run_and_capture_workflow(size_t index, Circuit *c, char *output, size_t capacity) {
    FILE *capture = tmpfile();
    assert(capture != NULL);
    int saved_stdout = dup(STDOUT_FILENO);
    assert(saved_stdout >= 0);
    assert(dup2(fileno(capture), STDOUT_FILENO) >= 0);
    int result = workflow_run(index, c);
    fflush(stdout);
    assert(dup2(saved_stdout, STDOUT_FILENO) >= 0);
    close(saved_stdout);
    rewind(capture);
    size_t n = fread(output, 1, capacity - 1, capture);
    output[n] = '\0';
    fclose(capture);
    return result;
}

static void test_overlap_rejection_is_atomic(void) {
    Circuit *c = INIT_CIRCUIT(3);
    assert(circuit_place_gate(c, edit(CIRCUIT_CX, 0, 1, 1, 0)) == NULL);
    assert(circuit_place_gate(c, edit(CIRCUIT_X, 2, 1, -1, 0)) == NULL);

    const Operation *original = circuit_gate_at(c, 1, 1);
    assert(circuit_update_gate(c, 1, 1, edit(CIRCUIT_CX, 0, 1, 2, 0)) != NULL);
    assert(circuit_gate_at(c, 1, 1) == original);
    assert(circuit_place_gate(c, edit(CIRCUIT_X, 2, 1, -1, 0)) != NULL);
    assert(circuit_gate_at(c, 0, 1) != NULL);
    assert(circuit_gate_at(c, 1, 1) != NULL);
    assert(circuit_gate_at(c, 2, 1) != NULL);
    assert(circuit_gate_at(c, 2, 2) == NULL);
    FREE_CIRCUIT(c);
}

static void test_public_builders_and_workflows(void) {
    Circuit *c = INIT_CIRCUIT(2);
    Hadamard(c, 0);
    RZ(c, 1, 0.5f);
    CX(c, 0, 1);
    assert(circuit_gate_at(c, 0, 1) != NULL);
    assert(circuit_gate_at(c, 1, 1) != NULL);
    assert(circuit_gate_at(c, 0, 2) != NULL);
    assert(circuit_gate_at(c, 1, 2) != NULL);
    FREE_CIRCUIT(c);

    c = INIT_CIRCUIT(2);
    assert(workflow_run(SIZE_MAX, c) != 0);
    assert(workflow_run(workflow_count(), c) != 0);
    FREE_CIRCUIT(c);

    /* Every workflow simulates the caller's circuit without editing it. */
    assert(workflow_count() >= 3);
    c = INIT_CIRCUIT(2);
    Hadamard(c, 0);
    CX(c, 0, 1);
    const Operation *h_before = circuit_gate_at(c, 0, 1);
    const Operation *cx_control_before = circuit_gate_at(c, 0, 2);
    const Operation *cx_target_before = circuit_gate_at(c, 1, 2);
    char output[2048];
    for (size_t index = 0; index < workflow_count(); index++) {
        assert(workflow_name(index) != NULL);
        assert(run_and_capture_workflow(index, c, output, sizeof(output)) == 0);
        assert(strstr(output, "00\t\t50.0000") != NULL);
        assert(strstr(output, "11\t\t50.0000") != NULL);
    }
    assert(circuit_gate_at(c, 0, 1) == h_before);
    assert(circuit_gate_at(c, 0, 2) == cx_control_before);
    assert(circuit_gate_at(c, 1, 2) == cx_target_before);

    /* Deleting CX through its target leaves H in place and changes the result. */
    assert(circuit_remove_gate(c, 1, 2) == NULL);
    for (size_t index = 0; index < workflow_count(); index++) {
        assert(run_and_capture_workflow(index, c, output, sizeof(output)) == 0);
        assert(strstr(output, "00\t\t50.0000") != NULL);
        assert(strstr(output, "01\t\t50.0000") != NULL);
    }
    assert(circuit_gate_at(c, 0, 1) == h_before);
    FREE_CIRCUIT(c);
}

int main(void) {
    test_out_of_order_insert_and_edit();
    test_cx_target_removal_relinks_and_recomputes();
    test_overlap_rejection_is_atomic();
    test_update_relocates_across_wires();
    test_legacy_borrowed_inputs_remain_caller_owned();
    test_public_builders_and_workflows();
    puts("editor regression tests passed");
    return 0;
}
