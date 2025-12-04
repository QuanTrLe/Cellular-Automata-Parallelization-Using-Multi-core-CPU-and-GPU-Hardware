#include <iostream>
#include <vector>
#include <cstdlib> 
#include <ctime>   
#include <numeric> 

#include "../GOL_cl_neighbors.cpp"

using namespace std;

int main(int argc, char* argv[]) {
    // ---------------------------------------------------------
    // 1. CONFIGURATION
    // ---------------------------------------------------------
    // Fixed settings for this benchmark
    int generations = 1000;
    int gridSize = 512;

    // Benchmark Loop Settings: Grid Size
    int start = 1;
    int end = 10;
    int repeats = 15;


    // ---------------------------------------------------------
    // 2. DATA PREPARATION
    // ---------------------------------------------------------
    // We generate the random grid ONCE and reuse it for every test
    // to ensure consistency across benchmarks.
    vector<uint8_t> inputData(gridSize * gridSize);
    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < gridSize * gridSize; i++) {
        inputData[i] = (rand() % 2);
    }


    // ---------------------------------------------------------
    // 2. SETUP
    // ---------------------------------------------------------
    srand(static_cast<unsigned int>(time(0)));

    cout << "Running Neighborhood Size Test: start radius ";
    cout << start;
    cout << " to end radius ";
    cout << end << endl;

    try {
        // Loop from 1 to 10
        for (int radius = start; radius <= end; radius += 1) {

            double totalTime = 0.0;

            // --- BENCHMARK RUNS ---
            for (int r = 0; r < repeats; r++) {

                GOL_cl sim(generations, gridSize, radius);
                sim.setup_grid(inputData);

                // Run and accumulate time
                totalTime += sim.run_simulation();
            }

            // Calculate Mean
            double meanTime = totalTime / repeats;

            // Output in CSV format: "Size, Time"
            cout << "Neighborhood Radius: " << radius << " with mean time " << meanTime << endl;
        }

    }
    catch (const exception& e) {
        cerr << "CRITICAL ERROR: " << e.what() << endl;
        return 1;
    }

    cout << "\nBenchmark Complete." << endl;
    return 0;
}