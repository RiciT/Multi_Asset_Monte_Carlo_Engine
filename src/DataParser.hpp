#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

class DataParser {
public:
    static std::vector<double> parseCSVs(const std::string &fileName)
    {
        std::vector<double> data;
        std::ifstream ifs(fileName);

        if (!ifs.is_open()) { throw std::runtime_error("Could not open CSV file"); }

        std::string line, val;
        while (std::getline(ifs, line))
        {
            std::stringstream ls(line);
            while (std::getline(ls, val, ','))
            {
                try
                {
                    data.push_back(std::stod(val));
                }
                catch (...) //... matches exc of any type https://en.cppreference.com/w/cpp/language/catch.html
                {
                    //catching any trailing commas and spaces
                }
            }
        }
        return data;
    }
};