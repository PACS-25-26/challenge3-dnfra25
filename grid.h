#pragma once
#include <vector>

class Grid {
public:
    int n;
    double h;

    std::vector<double> u;
    std::vector<double> u_new;

    Grid(int n);

    inline int idx(int i, int j) const {
        return i * n + j;
    }

    void init();
};
