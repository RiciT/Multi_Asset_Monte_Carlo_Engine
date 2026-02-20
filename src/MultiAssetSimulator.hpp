#pragma once
#include <random>
#include <vector>

#include "Assets.hpp"

class MultiAssetSimulator {
public:
    explicit MultiAssetSimulator(int assets, const int steps, const double T, const std::vector<double>& choleskyL) // NOLINT(*-msc51-cpp)
        : numAssets(assets), numSteps(steps), normal_dist(0.0, 1.0), corrMatrix(choleskyL)
    {
        dt = T / steps;
        std::random_device rd;
        rng.seed(rd());
    }

    //single path for the entire basket
    std::vector<double> generatePath(const std::vector<Asset>& basket);

private:
    int numAssets;
    int numSteps;
    double dt;
    std::vector<double> corrMatrix;
    std::mt19937 rng; //Mersenne Twister random
    std::normal_distribution<double> normal_dist;
};
