#include <iostream>
#include <vector>
#include <cstdlib> 
#include <ctime>   
#include <numeric> 

#include "../GOL_serial_benchmark.cpp"

using namespace std;

int main(int argc, char* argv[]) {
    // ---------------------------------------------------------
    // 1. CONFIGURATION
    // ---------------------------------------------------------
    // Fixed settings for this benchmark
    int generations = 1000;

    // Benchmark Loop Settings: Grid Size
    int startGrid = 128;
    int endGrid = 2048;
    int stepGrid = 32;
    int repeats = 15;

    // ---------------------------------------------------------
    // 2. SETUP
    // ---------------------------------------------------------
    srand(static_cast<unsigned int>(time(0)));

    cout << "Running Grid Size Test: start grid size ";
    cout << startGrid << " x " << startGrid;
    cout << " to end grid size ";
    cout << endGrid << " x " << endGrid;
    cout << " with stepsize of " << stepGrid << endl;

    try {
        // Loop from 128 to 2048, stepping by 32
        for (int size = startGrid; size <= endGrid; size += stepGrid) {

            // --- REGENERATE DATA FOR NEW SIZE ---
            // The grid size changes every loop, so we need a new random vector
            vector<uint8_t> inputData(size * size);
            for (int i = 0; i < size * size; i++) {
                inputData[i] = (rand() % 2);
            }

            double totalTime = 0.0;

            // --- BENCHMARK RUNS ---
            for (int r = 0; r < repeats; r++) {

                GOL_serial sim(generations, size);
                sim.setup_grid(inputData);

                // Run and accumulate time
                totalTime += sim.run_simulation();
            }

            // Calculate Mean
            double meanTime = totalTime / repeats;

            // Output in CSV format: "Size, Time"
            cout << "Grid Size: " << size << " x " << size << " with mean time " << meanTime << endl;
        }

    }
    catch (const exception& e) {
        cerr << "CRITICAL ERROR: " << e.what() << endl;
        return 1;
    }

    cout << "\nBenchmark Complete." << endl;
    return 0;
}