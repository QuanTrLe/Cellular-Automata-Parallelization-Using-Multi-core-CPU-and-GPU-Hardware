#include <iostream>
#include <cstdlib>
#include "GOL_mp_ver.hpp"

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: .\GOL_mp_ver.exe <rows> <cols> <generations> <threads>\n";
        return 1;
    }

    int rows = std::atoi(argv[1]);
    int cols = std::atoi(argv[2]);
    int generations = std::atoi(argv[3]);
    int threads = std::atoi(argv[4]);

    return run(rows, cols, generations, threads);
}
