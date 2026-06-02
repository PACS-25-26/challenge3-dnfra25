# Parallel Jacobi Solver (MPI + OpenMP)

**Author:** Francesco Di Napoli

---

## Overview

This project implements a parallel solver for the 2D Laplace equation using the Jacobi iterative method:

-∇²u = f(x,y) in Ω = (0,1)²  
with homogeneous Dirichlet boundary conditions.

The numerical method is applied to the manufactured solution:

f(x,y) = 8π² sin(2πx) sin(2πy)

with exact solution:

u(x,y) = sin(2πx) sin(2πy)

The goal of this project is to study the performance and scalability of a hybrid MPI + OpenMP implementation.

---

## Project Structure

jacobi_solver/
├── main.cpp              # MPI + OpenMP Jacobi solver
├── grid.h / grid.cpp     # Grid data structure (flattened 2D domain)
├── mpi_utils.h           # MPI metadata (rank, size)
├── config.h              # Simulation parameters
├── Makefile              # Build system (C++20 + MPI + OpenMP)
├── solver                # Executable
├── test/
│   ├── run_tests.sh      # Automated scalability test
│   ├── plot.py           # Visualization scripts
│   ├── results.csv       # Raw performance data
│   ├── RESULT.md         # Performance analysis
│   ├── hw.info           # Hardware specification
│   ├── time.png
│   ├── speedup.png

---

## Numerical Method

The Jacobi method discretizes the Laplace equation using a 5-point stencil:

U(i,j)^(k+1) = 1/4 [U(i+1,j) + U(i-1,j) + U(i,j+1) + U(i,j-1) + h² f(i,j)]

### Properties:
- Second-order finite difference scheme
- Explicit iterative solver
- Naturally parallelizable stencil computation

---

## Parallelization Strategy

### MPI Domain Decomposition

- The domain is split along rows
- Each MPI rank owns a contiguous block of rows
- Load balancing is uniform across ranks

---

### Ghost Cell Exchange

Each iteration requires boundary communication:

- Non-blocking MPI_Isend / MPI_Irecv
- MPI_Waitall synchronization
- Exchange of halo rows between neighboring ranks

---

### OpenMP Parallelization

Inside each MPI process:

- Jacobi update is parallelized using OpenMP
- collapse(2) is used for 2D loop parallelism
- reduction is used for local error computation

---

## Convergence Criterion

The stopping criterion is based on the L2 norm:

e = sqrt( h * Σ (U^(k+1) - U^(k))² )

Global convergence is computed using:

MPI_Allreduce (sum over all ranks)

---

## Output Format

Each MPI rank writes its own VTK file:

solution_rank_X.vtk

These files can be visualized using ParaView or other VTK-compatible tools.

---

## Build Instructions

Requirements:
- MPI (mpic++)
- OpenMP support
- C++20 compiler

Compile:

make

This produces the executable:

./solver

---

## Run Simulation

Example execution with 4 MPI ranks:

mpirun -np 4 ./solver

The number of ranks can be adjusted depending on available hardware.

---

## Performance Testing

Run scalability tests:

cd test
./run_tests.sh

This generates:
- results.csv (execution times)

Plot results:

python3 plot.py

This generates:
- time.png
- speedup.png

---

## Configuration

All parameters are defined in `config.h`:

- Grid size n
- tolerance tol
- max iterations max_iter

---

## Results

Performance analysis, speedup computation, and discussion are provided in:

test/RESULTS.md

---

## Design Choices

### Flat memory layout
The grid is stored as a 1D vector to improve cache efficiency and simplify MPI communication.

---

### Double buffering
Two arrays are used:
- u (current solution)
- u_new (updated solution)

This ensures correctness of the Jacobi iteration.

---

### Hybrid parallel model
- MPI for distributed memory parallelism
- OpenMP for shared memory parallelism inside each rank

---

### Explicit ghost exchange
Halo rows are exchanged using non-blocking MPI communication to ensure correctness of the stencil computation.

---

## Conclusions

The solver correctly implements a hybrid MPI + OpenMP Jacobi method for the 2D Laplace equation.

Key results:
- Correct numerical convergence
- Good strong scaling up to 4 MPI ranks
- Expected efficiency loss due to communication overhead and synchronization costs

The project demonstrates a solid understanding of:
- Domain decomposition techniques
- Hybrid parallel programming models
- Stencil-based numerical solvers

---

## Future Improvements

- Overlap communication and computation (MPI + OpenMP overlap)
- Multigrid or Gauss-Seidel solvers for faster convergence
- Preconditioning techniques
- Reduced synchronization overhead in convergence check
