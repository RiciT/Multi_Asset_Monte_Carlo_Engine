#pragma once

#include <utility>

#include "MultiAssetSimulator.hpp"

class BasketCallOption {
public:
    explicit BasketCallOption(MultiAssetSimulator simulator)
        : simulator(std::move(simulator))
    {}

    double calculateBasketCallPrice(int numPaths, double strike, double T, double r, const std::vector<Asset>& basket)
    {
        double totalPayoff = 0.0;

        for (auto p = 0; p < numPaths; ++p)
        {
            std::vector<double> terminalPrices = simulator.generatePath(basket);

            double averagePrice = 0.0;
            for (auto prices : terminalPrices) averagePrice += prices;
            averagePrice /= basket.size();

            //standard call payoff is: max(avg - strikePrice, 0)
            double payoff = std::max(averagePrice - strike, 0.0);
            totalPayoff += payoff;
        }

        //average and discount back to present value
        double averagePayoff = totalPayoff / numPaths;
        //using formula V_0 * e^{-rT}
        return std::exp(-r * T) * averagePayoff;
    }
private:
    MultiAssetSimulator simulator;
};