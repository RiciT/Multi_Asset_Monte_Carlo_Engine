#include <iostream>
#include <sstream>
#include <chrono>
#include <ranges>

#include "LinearAlgebraProvider.hpp"
#include "MultiAssetSimulator.hpp"
#include "Pricer.hpp"
#include "DataParser.hpp"

using namespace std;

template <typename S>
ostream& operator<<(ostream& os,
                    const vector<S>& vector) {

    // Printing all the elements using <<
    for (auto i : vector)
        os << i << " ";
    return os;
}


int main(int argc, char *argv[]) {
    //init close
    auto start = std::chrono::high_resolution_clock::now();

    constexpr int numPaths = 100000;
    try {
        //parse generated csvs
        const std::vector<double> marketSpots = DataParser::parseCSVs("../data/market_spots.csv");
        const std::vector<double> marketVols = DataParser::parseCSVs("../data/market_vols.csv");
        const std::vector<double> marketCorrelation = DataParser::parseCSVs("../data/market_correlation.csv");

        //printing data
        auto avg = 0.0;
        for (const auto i : marketSpots) avg += i;
        std::cout << "current avg: " << avg / static_cast<double>(marketSpots.size()) << std::endl;
        std::cout << "spots: " << std::endl << marketSpots << std::endl <<"volatilities: " << std::endl ;
        std::cout << marketVols << std::endl << "correlation_matrix: "  ;
        for (const auto [i, val] : std::views::enumerate(marketCorrelation))
        { if (i % static_cast<typeof(i)>(sqrt(marketCorrelation.size())) == 0) std::cout << std::endl; std::cout << val << " ";  }
        std::cout << std::endl << std::endl;

        //only starting clock after prints
        start = std::chrono::high_resolution_clock::now();

        //setup
        const int numAssets = static_cast<int>(marketSpots.size());

        constexpr double riskFreeRate = 0.037; //using 3.7% as the approximate 3 month us treasury rate
        constexpr double timeToMaturity = 1.0;
        constexpr int numSteps = 252; //num of trading days in a year

        std::vector<Asset> basket;
        basket.reserve(numAssets); //avoid reallocs
        for (int i = 0; i < numAssets; ++i)
            basket.push_back({marketSpots[i], marketVols[i], riskFreeRate});

        auto startmat = std::chrono::high_resolution_clock::now();
        const std::vector<double> choleskyMatrix = LinearAlgebraProvider::cholesky(marketCorrelation, numAssets);
        auto endmat = std::chrono::high_resolution_clock::now();
        const auto diff = static_cast<double>((endmat-startmat).count()) / 1000000000.0;
        std::cout << "Wall-clock runtime for decomp took: " << diff << "s "<< std::endl;

        //running sim and pricing pricing
        constexpr double strikePrice = 75.0;
        const MultiAssetSimulator simulator(numAssets, numSteps, timeToMaturity, choleskyMatrix);
        const BasketCallOption basketCallModel(simulator);
        const double optionPrice = basketCallModel.calculateBasketCallPrice(numPaths, strikePrice, timeToMaturity, riskFreeRate, basket);

        std::cout << "Basket Option Price: $" << optionPrice << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    }
    const auto end = std::chrono::high_resolution_clock::now();
    const auto diff = static_cast<double>((end-start).count()) / 1000000000.0;
    std::cout << "Wall-clock runtime: " << diff << "s with " << numPaths << " paths." << std::endl;

    return 0;
}
