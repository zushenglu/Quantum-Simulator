# Quantum Simulator

An exploratory quantum-circuit simulator written in C while learning quantum computing fundamentals. It represents quantum state with complex amplitudes, builds gate matrices, and evolves a state vector through a circuit.

This is a learning prototype, not a production-ready simulator or a replacement for Qiskit. The code prioritizes making the underlying linear algebra explicit over performance, API polish, or broad feature coverage.

## Current functionality

- Circuit representation with depth-ordered operations
- Complex state-vector simulation
- Single-qubit gates:
  - Hadamard (`H`)
  - Pauli-X (`X`)
  - Z-axis rotation (`RZ`)
- Controlled-NOT (`CNOT` / `CX`)
- Basis-state probability output after simulation

`run.c` contains the current example circuit.

## Build

Requirements:

- GCC (or another C compiler with C complex-number support)
- `make`
- the standard math library (`libm`)

Build without running the example:

```bash
make myproj
```

Build and run the example circuit:

```bash
make
```

Remove generated objects and the executable:

```bash
make clean
```

## Project status and limitations

This repository intentionally remains an early exploration project. Current limitations include:

- The simulator constructs dense full-system matrices, so time and memory scale exponentially with qubit count.
- Gate coverage is intentionally small.
- It reports state-vector probabilities but does not provide a full measurement/sampling API.
- Correctness tests, benchmarks, robust memory management, and a cleaner public API are future work.
- Some experimental or incomplete functions remain in the codebase as part of the original learning path.

The main value of the project is the implementation work behind the abstraction: complex amplitudes, tensor products, gate matrices, controlled operations, and state-vector evolution in C.

## Repository structure

- `gate.c`, `gate.h` — gate-matrix definitions
- `circuit.c`, `circuit.h` — circuit and operation representation
- `simulator.c`, `simulator.h` — state-vector evolution and matrix operations
- `convertor.c`, `convertor.h` — basis-state probability formatting
- `run.c` — example circuit
