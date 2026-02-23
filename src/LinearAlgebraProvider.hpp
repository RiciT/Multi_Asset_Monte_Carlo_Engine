#pragma once
#include <complex>
#include <vector>

class LinearAlgebraProvider {
public:
    explicit LinearAlgebraProvider();

    //for the algorithm see and further improvements see: https://www.cs.utexas.edu/~flame/Notes/NotesOnCholReal.pdf
    static std::vector<double> cholesky(const std::vector<double> &matrix, const int N, const int it = 1)
    {
        std::vector res(N * N, 0.0);
        bool isPosDef = true;

        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j <= i; j++)
            {
                double s = 0;
                for (int k = 0; k < j; k++)
                    s += res[index_2d_to_1d(i, k, N)] * res[index_2d_to_1d(j, k, N)];

                if (i == j)
                {
                    const double val = matrix[index_2d_to_1d(i, i, N)] - s;
                    if (val <= 0)
                    {
                        if (it < 2) { isPosDef = false; break; }
                        throw std::runtime_error("Matrix still not posdef after Higham's algorithm");
                    }
                    res[index_2d_to_1d(i, j, N)] = std::sqrt(val);
                }
                else
                    res[index_2d_to_1d(i, j, N)] = 1.0 / res[index_2d_to_1d(j, j, N)] * (matrix[index_2d_to_1d(i, j, N)] - s);
            }
            if (!isPosDef) break;
        }

        //if not positive definite find nearest corr matrix and retry the decomp
        if (!isPosDef)
            res = cholesky(nearestCorrelationMatrix(matrix, N), N, it + 1);

        return res;
    }

private:
    //Adapting to cases where the correlation matrix isnt perfectly positive definite but has a slightly negative eValue in that case find closest matrix
    // https://math.stackexchange.com/questions/1098039/converting-a-matrix-to-the-nearest-positive-definite-matrix --->
    //      https://nhigham.com/2021/01/26/what-is-the-nearest-positive-semidefinite-matrix/
    //      https://nhigham.com/2021/02/16/diagonally-perturbing-a-symmetric-matrix-to-make-it-positive-definite/
    //Highams alternating projections algorithm - might need to look at newtons method later as a benchmark said its much faster for higher dimensions nag.com
    static std::vector<double> nearestCorrelationMatrix(const std::vector<double>& matrix, int n)
    {
        std::vector<double> Y = matrix;
        std::vector X(n*n, 0.0);
        std::vector deltaS(n*n, 0.0);
        std::vector R(n*n, 0.0);

        constexpr double tol = 1e-8;
        constexpr int maxIter = 200;

        for (auto iter = 0; iter < maxIter; ++iter)
        {
            //R_k = Y_{k-1} - \Delta S_{k-1}
            for (auto i = 0; i < n * n; ++i)
                R[i] = Y[i] - deltaS[i];

            //X_k = P_S(R_k) - project onto positive semi-def
            X = projectPosSemiDef(R, n);

            //\Delta S_k = X_k - R_k
            for (auto i = 0; i < n * n; ++i)
                deltaS[i] = X[i] - R[i];

            //Y_k = P_U(X_k) - project it back onto unit diagonal
            std::vector<double> Y_old = Y;
            Y = X;
            for (auto i = 0; i < n * n; ++i)
                Y[index_2d_to_1d(i, i, n)] = 1.0;

            //check for convergence
            double maxDiff = 0;
            for (auto i = 0; i < n * n; ++i)
                maxDiff = std::max(maxDiff, std::abs(Y_old[i] - Y[i]));
            if (maxDiff < tol) break;
        }

        return Y;
    }

    //projects matrix onto pos semi definite space
    static std::vector<double> projectPosSemiDef(const std::vector<double> &mat, const int n)
    {
        std::vector<double> eigenVals;
        std::vector<double> eigenVecs;
        jacobiEigenDecomposition(mat, n, eigenVals, eigenVecs);

        //reconstruct X = V * max(D, 1e-8) * V^T
        //using 1e-8 instead of 0.0 to ensure strictly positive definite for Cholesky
        std::vector res(n * n, 0.0);
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                double sum = 0.0;
                for (int k = 0; k < n; ++k)
                {
                    const double val = std::max(eigenVals[k], 1e-8);
                    sum += eigenVecs[index_2d_to_1d(i, k, n)] * val * eigenVecs[index_2d_to_1d(j, k, n)];
                }
                res[index_2d_to_1d(i, j, n)] = sum;
            }
        }
        return res;
    }

    //helpers
    static int index_2d_to_1d(const int i, const int j, const int N)
    {
        return i * N + j;
    }

    //jacobi eigenvalue decomp algorithm for symmetric matrices
    //see:https://arxiv.org/pdf/2105.14642
    static void jacobiEigenDecomposition(std::vector<double> A, const int N, std::vector<double>& eigenVals, std::vector<double>& eigenVecs)
    {
        eigenVecs.assign(N * N, 0.0);
        for (int i = 0; i < N; ++i) eigenVecs[index_2d_to_1d(i, i, N)] = 1.0;

        constexpr int maxSweeps = 50;
        for (int sweep = 0; sweep < maxSweeps; ++sweep)
        {
            double maxOffDiag = 0.0;
            int p = 0, q = 0;

            for (int i = 0; i < N; ++i)
            {
                for (int j = i + 1; j < N; ++j)
                {
                    if (const double absVal = std::abs(A[index_2d_to_1d(i, j, N)]); absVal > maxOffDiag) {
                        maxOffDiag = absVal;
                        p = i;
                        q = j;
                    }
                }
            }

            if (maxOffDiag < 1e-10) break;

            const double app = A[index_2d_to_1d(p, p, N)];
            const double aqq = A[index_2d_to_1d(q, q, N)];
            const double apq = A[index_2d_to_1d(p, q, N)];

            const double tau = (aqq - app) / (2.0 * apq);
            const double t = (tau >= 0) ? 1.0 / (tau + std::sqrt(1.0 + tau * tau)) : -1.0 / (-tau + std::sqrt(1.0 + tau * tau));
            const double c = 1.0 / std::sqrt(1.0 + t * t);
            const double s = t * c;

            for (int i = 0; i < N; ++i)
            {
                if (i != p && i != q)
                {
                    const double aip = A[index_2d_to_1d(i, p, N)];
                    const double aiq = A[index_2d_to_1d(i, q, N)];
                    A[index_2d_to_1d(i, p, N)] = A[index_2d_to_1d(p, i, N)] = c * aip - s * aiq;
                    A[index_2d_to_1d(i, q, N)] = A[index_2d_to_1d(q, i, N)] = s * aip + c * aiq;
                }
                const double vip = eigenVecs[index_2d_to_1d(i, p, N)];
                const double viq = eigenVecs[index_2d_to_1d(i, q, N)];
                eigenVecs[index_2d_to_1d(i, p, N)] = c * vip - s * viq;
                eigenVecs[index_2d_to_1d(i, q, N)] = s * vip + c * viq;
            }

            A[index_2d_to_1d(p, p, N)] = app - t * apq;
            A[index_2d_to_1d(q, q, N)] = aqq + t * apq;
            A[index_2d_to_1d(p, q, N)] = A[index_2d_to_1d(q, p, N)] = 0.0;
        }

        eigenVals.resize(N);
        for (int i = 0; i < N; ++i)
            eigenVals[i] = A[index_2d_to_1d(i, i, N)];
    }
};