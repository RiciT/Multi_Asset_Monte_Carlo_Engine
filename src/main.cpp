#include <cstdio>
#include <iostream>
#include <sstream>

#include "LinearAlgebraProvider.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  std::vector<double> mat = {5,3,2,5};

  mat = LinearAlgebraProvider::cholesky(mat, 2);
  stringstream ss;

  for (const double i : mat)
    ss << i << " ";

  std::cout << ss.str() << std::endl;

  return 0;
}
