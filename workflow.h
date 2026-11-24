#ifndef QSIM_WORKFLOW_H
#define QSIM_WORKFLOW_H

#include <stddef.h>
#include "circuit.h"

size_t workflow_count(void);
const char *workflow_name(size_t index);
/* Runs against the caller's circuit and returns zero on success. */
int workflow_run(size_t index, Circuit *circuit);

#endif
