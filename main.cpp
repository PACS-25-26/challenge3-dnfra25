#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include "config.h"
#include "mpi_utils.h"

constexpr double PI = 3.141592653589793;

double f(double x, double y) {
    return 8.0 * PI * PI *
           std::sin(2.0 * PI * x) *
           std::sin(2.0 * PI * y);
}

void compute_decomposition(int n, int size, int rank,
                           int &start, int &end) {
    int base = n / size;
    int rem = n % size;

    start = rank * base + std::min(rank, rem);
    end = start + base - 1;
    if (rank < rem) end++;
}

inline int idx(int i, int j, int n) {
    return i * n + j;
}

// =====================================================
// VTK OUTPUT
// =====================================================
void write_vtk(const std::vector<double>& u,
               int n,
               int local_n,
               int start,
               int rank) {

    std::ofstream out("solution_rank_" + std::to_string(rank) + ".vtk");

    out << "# vtk DataFile Version 3.0\n";
    out << "Jacobi solution\n";
    out << "ASCII\n";
    out << "DATASET STRUCTURED_GRID\n";

    out << "DIMENSIONS " << n << " " << (local_n + 2) << " 1\n";
    out << "POINTS " << n * (local_n + 2) << " float\n";

    double h = 1.0 / (n - 1);

    for (int i = 0; i < local_n + 2; i++) {
        for (int j = 0; j < n; j++) {
            out << j * h << " " << (start + i - 1) * h << " 0\n";
        }
    }

    out << "POINT_DATA " << n * (local_n + 2) << "\n";
    out << "SCALARS u double\n";
    out << "LOOKUP_TABLE default\n";

    for (double v : u) {
        out << v << "\n";
    }

    out.close();
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Config cfg;

    int n = cfg.n;
    double h = 1.0 / (n - 1);

    int start, end;
    compute_decomposition(n, size, rank, start, end);

    int local_n = end - start + 1;

    if (rank == 0) {
        std::cout << "Running with " << size << " MPI ranks\n";
        std::cout << "Grid size: " << n << "x" << n << "\n";
        std::cout << "OpenMP threads: " << omp_get_max_threads() << "\n";
    }

    std::vector<double> u((local_n + 2) * n, 0.0);
    std::vector<double> u_new((local_n + 2) * n, 0.0);

    int iter = 0;
    double error = 1.0;

    double t0 = MPI_Wtime();

    while (error > cfg.tol && iter < cfg.max_iter) {

        // =====================================================
        // GHOST EXCHANGE (SAFE MPI)
        // =====================================================
        std::vector<MPI_Request> reqs;

        if (rank > 0) {
            MPI_Request r;
            MPI_Irecv(&u[idx(0,0,n)], n, MPI_DOUBLE,
                      rank - 1, 1, MPI_COMM_WORLD, &r);
            reqs.push_back(r);
        }

        if (rank < size - 1) {
            MPI_Request r;
            MPI_Irecv(&u[idx(local_n+1,0,n)], n, MPI_DOUBLE,
                      rank + 1, 0, MPI_COMM_WORLD, &r);
            reqs.push_back(r);
        }

        if (rank > 0) {
            MPI_Request r;
            MPI_Isend(&u[idx(1,0,n)], n, MPI_DOUBLE,
                      rank - 1, 0, MPI_COMM_WORLD, &r);
            reqs.push_back(r);
        }

        if (rank < size - 1) {
            MPI_Request r;
            MPI_Isend(&u[idx(local_n,0,n)], n, MPI_DOUBLE,
                      rank + 1, 1, MPI_COMM_WORLD, &r);
            reqs.push_back(r);
        }

        MPI_Waitall(reqs.size(), reqs.data(), MPI_STATUSES_IGNORE);

        // =====================================================
        // JACOBI + OPENMP
        // =====================================================
        double local_error = 0.0;

        #pragma omp parallel for collapse(2) reduction(+:local_error)
        for (int i = 1; i <= local_n; i++) {
            for (int j = 1; j < n - 1; j++) {

                double x = (start + i - 1) * h;
                double y = j * h;

                double new_val =
                    0.25 * (
                        u[idx(i-1,j,n)] +
                        u[idx(i+1,j,n)] +
                        u[idx(i,j-1,n)] +
                        u[idx(i,j+1,n)] +
                        h * h * f(x,y)
                    );

                u_new[idx(i,j,n)] = new_val;

                double diff = new_val - u[idx(i,j,n)];
                local_error += diff * diff;
            }
        }

        MPI_Allreduce(&local_error, &error, 1,
                      MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        error = std::sqrt(h * error);

        u.swap(u_new);

        iter++;
    }

    double t1 = MPI_Wtime();

    if (rank == 0) {
        std::cout << "\n=== FINAL RESULT ===\n";
        std::cout << "Iterations: " << iter << "\n";
        std::cout << "Final error: " << error << "\n";
        std::cout << "Time: " << (t1 - t0) << " s\n";
    }

    // =====================================================
    // VTK OUTPUT (one file per rank)
    // =====================================================
    write_vtk(u, n, local_n, start, rank);

    MPI_Finalize();
    return 0;
}
