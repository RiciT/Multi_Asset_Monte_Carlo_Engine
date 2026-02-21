#include <iostream>
#include <sstream>
#include <ctime>

#include "LinearAlgebraProvider.hpp"
#include "MultiAssetSimulator.hpp"
#include "Pricer.hpp"
#include "DataParser.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    clock_t tStart = clock();

    try {
        //parse generated csvs
        const std::vector<double> marketSpots = DataParser::parseCSVs("../data/market_spots.csv");
        const std::vector<double> marketVols = DataParser::parseCSVs("../data/market_vols.csv");
        const std::vector<double> marketCorrelation = DataParser::parseCSVs("../data/market_correlation.csv");

        //setup
        const int numAssets = static_cast<int>(marketSpots.size());

        double riskFreeRate = 0.037; //using 3.7% as the approximate 3 month us treasury rate
        double timeToMaturity = 1.0;
        constexpr int numSteps = 252; //num of trading days in a year
        int numPaths = 100000;

        std::vector<Asset> basket;
        basket.reserve(numAssets); //avoid reallocs
        for (int i = 0; i < numAssets; ++i) {
            basket.push_back({marketSpots[i], marketVols[i], riskFreeRate});
        }
        std::vector<double> choleskyMatrix = LinearAlgebraProvider::cholesky(marketCorrelation, numAssets);

        //running sim and pricing pricing
        constexpr double strikePrice = 200.0;
        const MultiAssetSimulator simulator(numAssets, numSteps, timeToMaturity, choleskyMatrix);
        BasketCallOption basketCallModel(simulator);
        const double optionPrice = basketCallModel.calculateBasketCallPrice(numPaths, strikePrice, timeToMaturity, riskFreeRate, basket);

        std::cout << "Basket Option Price: $" << optionPrice << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }

    printf("Time taken: %.2fs\n", static_cast<double>(clock() - tStart)/CLOCKS_PER_SEC);
    return 0;
}
