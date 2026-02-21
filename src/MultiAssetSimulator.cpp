#include <ranges>
#include <utility>

#include "MultiAssetSimulator.hpp"

std::vector<double> MultiAssetSimulator::generatePath(const std::vector<Asset> &basket,
    std::mt19937 &localRng, std::normal_distribution<>& n_dist) const
{
    std::vector<double> currentPrices(numAssets);
    //we work in log-space to avoid floating point errors
    for (auto i = 0; i < numAssets; ++i) currentPrices[i] = std::log(basket[i].spot);

    std::vector<double> Z(numAssets);
    std::vector<double> X(numAssets);

    for (auto t = 0; t < numSteps; ++t)
    {
        //Gen independent draws of standard normal
        for (auto i = 0; i < numAssets; ++i) Z[i] = n_dist(localRng);

        //Apply correlation matrix
        for (auto i = 0; i < numAssets; ++i)
        {
            X[i] = 0.0;
            for (auto j = 0; j <= i; ++j)
                X[i] += corrMatrix[i * numAssets + j] * Z[j];
        }

        //Update prices using the stochastic formula with gbm
        //S_{i, t + \delta t} = S_{i, t} * e^{(r - 0.5 \sigma_i^2)\delta t + \sigma_i \sqrt{\delta t}\dot X_i}
        for (auto i = 0; i < numAssets; ++i)
        {
            const double drift = (basket[i].risk_free_rate - 0.5 * basket[i].volatility * basket[i].volatility) * dt;
            const double diffusion = basket[i].volatility * sqrt(dt) * X[i];
            currentPrices[i] += drift + diffusion;
        }
    }

    for (auto i = 0; i < numAssets; ++i) currentPrices[i] = std::exp(currentPrices[i]);

    return currentPrices;
}
