#pragma once

#include <utility>
#include <omp.h>

#include "MultiAssetSimulator.hpp"
#include "Assets.hpp"

class BasketCallOption {
public:
    explicit BasketCallOption(MultiAssetSimulator simulator)
        : simulator(std::move(simulator))
    {}

    [[nodiscard]] double calculateBasketCallPrice(const int numPaths, const double strike, const double T,
        const double r, const std::vector<Asset>& basket) const
    {
        const int numAssets = static_cast<int>(basket.size());
        const double dt = T / static_cast<double>(simulator.getNumSteps());

        //precompute all static simulation parameters to eliminate redundant inner-loop math
        std::vector<PrecomputedAsset> precompBasket(numAssets);

        for (int i = 0; i < numAssets; ++i)
        {
            precompBasket[i].logSpot = std::log(basket[i].spot);
            precompBasket[i].drift = (basket[i].risk_free_rate - 0.5 * basket[i].volatility * basket[i].volatility) * dt;
            precompBasket[i].diffusion = basket[i].volatility * std::sqrt(dt);
        }

        double totalPayoff = 0.0;

        #pragma omp parallel shared(numPaths, strike, basket, numAssets, precompBasket) reduction(+:totalPayoff)
        {
            //thread local rng - we need unique seeds (using thread id) so the streams dont overlap
            const int threadId = omp_get_thread_num();
            std::mt19937 localRng(42 + threadId); //mersenne twister random
            std::normal_distribution normalDist(0.0, 1.0);

            std::vector<double> currentPrices(numAssets);
            std::vector<double> Z(numAssets);
            std::vector<double> X(numAssets);

            //disabled warnings are because it does not recognise (because of omp) that they are being used
            #pragma omp for
            // ReSharper disable once CppDFALoopConditionNotUpdated
            // ReSharper disable once CppDFAUnreadVariable
            for (auto p = 0; p < numPaths; ++p)
            {
                simulator.generatePath(precompBasket, localRng, normalDist, currentPrices, Z, X);

                // ReSharper disable once CppDFAUnreadVariable
                double averagePrice = 0.0;
                // ReSharper disable once CppDFALoopConditionNotUpdated
                // ReSharper disable once CppDFAUnreadVariable
                for (int i = 0; i < numAssets; ++i)
                {
                    averagePrice += currentPrices[i];
                }
                averagePrice /= static_cast<double>(numAssets);

                // ReSharper disable once CppDFAUnusedValue
                // ReSharper disable once CppDFAUnreadVariable
                const double payoff = std::max(averagePrice - strike, 0.0);
                totalPayoff += payoff;
            }
        }
        //average and discount back to present value
        const double averagePayoff = totalPayoff / numPaths;
        //using formula V_0 * e^{-rT}
        return std::exp(-r * T) * averagePayoff;
    }
private:
    MultiAssetSimulator simulator;
};