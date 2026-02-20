#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "LinearAlgebraProvider.hpp"
#include "MultiAssetSimulator.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    int numAssets = 2;
    int numSteps = 252;  //trading days in a year
    double T = 1.0;
    double r = 0.05;

    std::vector<Asset> assets = {
      {100.0, 0.20, r},
      {100.0, 0.25, r}
    };

    std::vector<double> correlationMatrix = {
        1.0, -0.7,
        -0.7,  1.0
    };

    // 3. Compute Cholesky from scratch
    std::vector<double> L;
    try {
        L = LinearAlgebraProvider::cholesky(correlationMatrix, numAssets);
    } catch (const std::exception& e) {
        std::cerr << "Math Error: " << e.what() << std::endl;
        return 1;
    }

    // 4. Run Monte Carlo Simulation
    MultiAssetSimulator sim(numAssets, numSteps, T, L);

    int numPaths = 100000;
    std::vector<double> sumTerminalPrices(numAssets, 0.0);
    double correlationSum = 0.0;

    std::cout << "Simulating " << numPaths << " paths..." << std::endl;

    for (int p = 0; p < numPaths; ++p) {
        std::vector<double> terminalPrices = sim.generatePath(assets);
        for (int i = 0; i < numAssets; ++i) {
            sumTerminalPrices[i] += terminalPrices[i];
        }
    }

    // 5. Verification
    double expectedPrice = assets[0].spot * std::exp(r * T);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "--- RESULTS ---" << std::endl;
    std::cout << "Theoretical Forward Price: " << expectedPrice << std::endl;
    for (int i = 0; i < numAssets; ++i) {
        double avg = sumTerminalPrices[i] / numPaths;
        std::cout << "Asset " << i << " Simulated Avg: " << avg
                 << " (Error: " << std::abs(avg - expectedPrice) << ")" << std::endl;
    }

    return 0;
}
