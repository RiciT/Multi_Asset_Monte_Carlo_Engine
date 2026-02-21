#pragma once

#include <utility>
#include <omp.h>

#include "MultiAssetSimulator.hpp"

class BasketCallOption {
public:
    explicit BasketCallOption(MultiAssetSimulator simulator)
        : simulator(std::move(simulator))
    {}

    [[nodiscard]] double calculateBasketCallPrice(const int numPaths, const double strike, const double T,
        const double r, const std::vector<Asset>& basket) const
    {
        double totalPayoff = 0.0;

        #pragma omp parallel default(none) shared(numPaths, strike, basket) reduction(+:totalPayoff)
        {
            //thread local rng - we need unique seeds (using thread id) so the streams dont overlap
            const int threadId = omp_get_thread_num();
            std::mt19937 localRng(42 + threadId); //mersenne twister random
            std::normal_distribution normalDist(0.0, 1.0);

            const int numAssets = static_cast<int>(basket.size());
            std::vector<double> currentPrices(numAssets);
            std::vector<double> Z(numAssets);
            std::vector<double> X(numAssets);

            //disabled warnings are because it does not recognise (because of omp) that they are being used
            #pragma omp for
            // ReSharper disable once CppDFALoopConditionNotUpdated
            // ReSharper disable once CppDFAUnreadVariable
            for (auto p = 0; p < numPaths; ++p)
            {
                simulator.generatePath(basket, localRng, normalDist, currentPrices, Z, X);

                // ReSharper disable once CppDFAUnreadVariable
                double averagePrice = 0.0;
                // ReSharper disable once CppDFALoopConditionNotUpdated
                // ReSharper disable once CppDFAUnreadVariable
                for (int i = 0; i < numAssets; ++i) {
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