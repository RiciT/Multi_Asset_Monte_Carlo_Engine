#include <ranges>

#include "MultiAssetSimulator.hpp"

#include <utility>

std::vector<double> MultiAssetSimulator::generatePath(const std::vector<Asset> &basket)
{
    std::vector<double> currentPrices(numAssets);
    //we work in log-space to avoid floating point errors
    for (auto i = 0; i < numAssets; ++i) currentPrices[i] = std::log(basket[i].spot);

    for (auto t = 0; t < numSteps; ++t)
    {
        //Gen independent draws of standard normal
        std::vector<double> Z(numAssets);
        for (auto i = 0; i < numAssets; ++i) Z[i] = normal_dist(rng);

        //Apply correlation matrix
        std::vector<double> X(numAssets, 0.0);
        for (auto i = 0; i < numAssets; ++i)
            for (auto j = 0; j <= i; ++j)
                X[i] += corrMatrix[i * numAssets + j] * Z[j];

        //Update prices using the stochastic formula with gbm
        //S_{i, t + \delta t} = S_{i, t} * e^{(r - 0.5 \sigma_i^2)\delta t + \sigma_i \sqrt{\delta t}\dot X_i}
        for (auto i = 0; i < numAssets; ++i)
        {
            double drift = (basket[i].risk_free_rate - 0.5 * basket[i].volatility * basket[i].volatility) * dt;
            double diffusion = basket[i].volatility * sqrt(dt) * X[i];
            currentPrices[i] += drift + diffusion;
        }
    }

    for (auto i = 0; i < numAssets; ++i) currentPrices[i] = std::exp(currentPrices[i]);

    return currentPrices;
}
