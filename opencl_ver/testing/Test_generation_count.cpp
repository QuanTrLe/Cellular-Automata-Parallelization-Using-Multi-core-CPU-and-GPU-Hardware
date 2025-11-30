#include <iostream>
#include <vector>
#include <cstdlib> // for rand() and atoi()
#include <ctime>   // for seeding random
#include <numeric> // for average calculation

// Include your implementation file directly
// Ensure GOL_cl_ver.cpp is "Excluded from Build" in Visual Studio properties
#include "../GOL_cl_ver.cpp"

using namespace std;

int main(int argc, char* argv[]) {
    // ---------------------------------------------------------
    // 1. CONFIGURATION
    // ---------------------------------------------------------
    // Fixed settings for this benchmark
    int gridSize = 512;
    int workGroupSize = 16;

    // Benchmark Loop Settings
    int startGen = 1;
    int endGen = 10000;
    int stepGen = 1000;
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
    // 3. EXECUTION LOOP
    // ---------------------------------------------------------

    // Print CSV-style header for easy copying to Excel
    cout << "\nGenerations, Mean_Time_Seconds" << endl;

    try {
        // Loop over generation counts: 1, 101, 201, ...
        // the weird step count is just to -1 so that it goes to 100 instead of 101
        for (int gen = startGen; gen <= endGen; gen += (stepGen + (-1 * (gen == 1)))) {

            double totalTime = 0.0;

            // Run X times for this specific config
            for (int r = 0; r < repeats; r++) {

                // Instantiate NEW simulation for every run to reset state
                GOL_cl sim(gen, gridSize, workGroupSize);

                // Load the pre-generated random data
                sim.setup_grid(inputData);

                // Run and accumulate time
                // (sim.run_simulation() returns double seconds as per your previous code)
                totalTime += sim.run_simulation();
            }

            // Calculate Mean
            double meanTime = totalTime / repeats;

            // Output in CSV format: "Gen, Time"
            cout << "Running " << gen << " generations, mean time: " << meanTime << endl;
        }

    }
    catch (const exception& e) {
        cerr << "CRITICAL ERROR: " << e.what() << endl;
        return 1;
    }

    cout << "\nBenchmark Complete." << endl;
    return 0;
}