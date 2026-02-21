#pragma once

#include <utility>
#include <omp.h>

#include "MultiAssetSimulator.hpp"

class BasketCallOption {
public:
    explicit BasketCallOption(MultiAssetSimulator simulator)
        : simulator(std::move(simulator))
    {}

    double calculateBasketCallPrice(const int numPaths, const double strike, const double T, const double r, const std::vector<Asset>& basket)
    {
        double totalPayoff = 0.0;

        #pragma omp paralell for reduction (+:totalPayoff)
        for (auto p = 0; p < numPaths; ++p)
        {
            //thread loccal rng - we need unique seeds (using thread id) so the streams dont overlap
            std::mt19937 localRng(42 + omp_get_thread_num() * 1000 + p);
            const std::normal_distribution normalDist(0.0, 1.0);

            std::vector<double> terminalPrices = simulator.generatePath(basket, localRng, normalDist);

            double averagePrice = 0.0;
            for (const auto prices : terminalPrices) averagePrice += prices;
            averagePrice /= static_cast<double>(basket.size());

            //standard call payoff is: max(avg - strikePrice, 0)
            const double payoff = std::max(averagePrice - strike, 0.0);
            totalPayoff += payoff;
        }

        //average and discount back to present value
        const double averagePayoff = totalPayoff / numPaths;
        //using formula V_0 * e^{-rT}
        return std::exp(-r * T) * averagePayoff;
    }
private:
    MultiAssetSimulator simulator;
};