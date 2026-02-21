#pragma once
#include <complex>
#include <vector>

class LinearAlgebraProvider {
public:
    explicit LinearAlgebraProvider();

    //TODO: Adapt to cases where the correlation matrix isnt perfectly positive definite but has a slightly negative eValue in that case find closest matrix
    // https://math.stackexchange.com/questions/1098039/converting-a-matrix-to-the-nearest-positive-definite-matrix --->
    //      https://nhigham.com/2021/01/26/what-is-the-nearest-positive-semidefinite-matrix/
    //      https://nhigham.com/2021/02/16/diagonally-perturbing-a-symmetric-matrix-to-make-it-positive-definite/

    //For the algorithm see and further improvements see: https://www.cs.utexas.edu/~flame/Notes/NotesOnCholReal.pdf
    static std::vector<double> cholesky(const std::vector<double> &matrix, const int N)
    {
        std::vector res(N * N, 0.0);

        for (int i = 0; i < N; i++) {
            for (int j = 0; j <= i; j++) {
                double s = 0;
                for (int k = 0; k < j; k++)
                    s += res[index_2d_to_1d(i, k, N)] * res[index_2d_to_1d(j, k, N)];

                if (i == j) {
                    const double val = matrix[index_2d_to_1d(i, i, N)] - s;
                    if (val <= 0) throw std::runtime_error("Matrix not Positive Definite");
                    res[index_2d_to_1d(i, j, N)] = std::sqrt(val);
                }
                else
                    res[index_2d_to_1d(i, j, N)] = 1.0 / res[index_2d_to_1d(j, j, N)] * (matrix[index_2d_to_1d(i, j, N)] - s);
            }
        }
        return res;
    }

private:
    static int index_2d_to_1d(const int i, const int j, const int N)
    {
        return i * N + j;
    }
};