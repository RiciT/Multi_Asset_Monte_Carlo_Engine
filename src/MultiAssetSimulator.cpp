#include <ranges>
#include <utility>

#include "MultiAssetSimulator.hpp"

void MultiAssetSimulator::generatePath(const std::vector<PrecomputedAsset> &basket,
                      std::mt19937 &localRng, std::normal_distribution<>& n_dist,
                      std::vector<double>& currentPrices, std::vector<double>& Z, std::vector<double>& X) const
{
    //we work in log-space to avoid floating point errors
    for (auto i = 0; i < numAssets; ++i) currentPrices[i] = basket[i].logSpot;

    for (auto t = 0; t < numSteps; ++t)
    {
        //gen independent draws of standard normal
        for (auto i = 0; i < numAssets; ++i) Z[i] = n_dist(localRng);

        //apply correlation matrix
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
            currentPrices[i] += basket[i].drift + basket[i].diffusion * X[i];
        }
    }

    for (auto i = 0; i < numAssets; ++i) currentPrices[i] = std::exp(currentPrices[i]);
}
