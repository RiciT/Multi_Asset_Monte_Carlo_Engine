#pragma once
#include <complex>
#include <vector>

class LinearAlgebraProvider {
public:
    explicit LinearAlgebraProvider();

    static std::vector<double> cholesky(std::vector<double> &matrix, int N)
    {
        for (auto i = 0; i < N; i++)
        {
            matrix[index_2d_to_1d(i,i,N)] = std::sqrt(matrix[index_2d_to_1d(i,i,N)]);

            for (auto j = i; j < N; j++)
                matrix[index_2d_to_1d(j,i,N)] /= matrix[index_2d_to_1d(i,i,N)];
            for (auto k = i; k < N; k++)
                for (auto j = k - 1; j < N; j++)
                    matrix[index_2d_to_1d(j,k,N)] -= matrix[index_2d_to_1d(j, i, N)] * matrix[index_2d_to_1d(k, i, N)];
        }

        return matrix;
    }

private:
    static int index_2d_to_1d(const int i, const int j, const int N)
    {
        return i * N + j;
    }

};