#include "grid.h"

Grid::Grid(int n_) : n(n_) {
    h = 1.0 / (n - 1);

    u.assign(n * n, 0.0);
    u_new.assign(n * n, 0.0);
}

void Grid::init() {
    // Dirichlet = 0 su tutti i bordi → già inizializzato
}
