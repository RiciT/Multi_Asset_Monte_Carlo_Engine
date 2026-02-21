#pragma once
#include <random>
#include <vector>

#include "Assets.hpp"

class MultiAssetSimulator {
public:
    explicit MultiAssetSimulator(const int assets, const int steps, const double T, const std::vector<double>& choleskyL) // NOLINT(*-msc51-cpp)
        : numAssets(assets), numSteps(steps), dt(T / steps), corrMatrix(choleskyL)
    {}

    //single path for the entire basket
    void generatePath(const std::vector<Asset> &basket, std::mt19937& localRng, std::normal_distribution<>& n_dist,
        std::vector<double>& currentPrices, std::vector<double>& Z, std::vector<double>& X) const;

private:
    int numAssets;
    int numSteps;
    double dt;
    std::vector<double> corrMatrix;
};
