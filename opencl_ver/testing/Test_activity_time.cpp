#include <iostream>
#include <vector>
#include <cstdlib> 
#include <ctime>   
#include <numeric> 

#include "../GOL_cl_ver.cpp"

int main(int argc, char* argv[]) {
    // ---------------------------------------------------------
    // 1. CONFIGURATION
    // ---------------------------------------------------------
    // Fixed settings
    int gridSize = 512;
    int workGroupSize = 16;
    int generations = 1000;

    // Benchmark Loop Settings: Density
    int startDensity = 0;   // 0%
    int endDensity = 100;   // 100%
    int stepDensity = 1;    // 1% steps
    int repeats = 15;

    // ---------------------------------------------------------
    // 2. SETUP
    // ---------------------------------------------------------
    srand(static_cast<unsigned int>(time(0)));

    cout << "Running grid density time to give meantime, starting with ";
    cout << startDensity << "% and ending with " << endDensity;
    cout << "% with steps of " << stepDensity << "% each time" << endl;

    try {
        // Loop from 0% to 100%
        for (int p = startDensity; p <= endDensity; p += stepDensity) {

            // --- GENERATE DATA FOR SPECIFIC DENSITY ---
            vector<uint8_t> inputData(gridSize * gridSize);

            for (int i = 0; i < gridSize * gridSize; i++) {
                // Generate a random number between 0 and 99.
                // If it is less than 'p', the cell is alive.
                // Ex: if p=0, (rand < 0) is never true -> 0% alive
                // Ex: if p=100, (rand < 100) is always true -> 100% alive
                int chance = rand() % 100;
                inputData[i] = (chance < p) ? 1 : 0;
            }

            double totalTime = 0.0;

            // --- BENCHMARK RUNS ---
            for (int r = 0; r < repeats; r++) {

                // New simulation instance
                GOL_cl sim(generations, gridSize, workGroupSize);
                sim.setup_grid(inputData);

                // Run and accumulate time
                totalTime += sim.run_simulation();
            }

            // Calculate Mean
            double meanTime = totalTime / repeats;

            // Output in CSV format: "Percent, Time"
            cout << "Percentage " << p << "% gives meantime " << meanTime << endl;
        }

    }
    catch (const exception& e) {
        cerr << "CRITICAL ERROR: " << e.what() << endl;
        return 1;
    }

    cout << "\nBenchmark Complete." << endl;
    return 0;
}