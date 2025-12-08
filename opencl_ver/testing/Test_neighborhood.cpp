#include <iostream>
#include <vector>
#include <cstdlib> 
#include <ctime>   
#include <numeric> 
#include <iomanip> // for setprecision

// Ensure this file exists and the class inside is named 'GOL_cl'
// The class constructor must accept (generations, gridSize, radius)
#include "../GOL_cl_neighbors.cpp"

using namespace std;

int main(int argc, char* argv[]) {
    // ---------------------------------------------------------
    // 1. CONFIGURATION
    // ---------------------------------------------------------
    int gridSize = 512;
    int generations = 1000;
    int repeats = 15;

    // Target densities for neighborhoods 1 through 10
    // Index 0 unused, Index 1 is 50%, Index 2 is 24.57%, etc.
    const double targetDensities[] = {
        0.0, 50.0, 24.57, 14.46, 9.59, 6.89, 5.17, 4.03, 3.23, 2.64, 2.21
    };

    srand(static_cast<unsigned int>(time(0)));

    cout << "========================================" << endl;
    cout << "   OpenCL Neighborhood Benchmark        " << endl;
    cout << "========================================" << endl;
    cout << "Grid Size:   " << gridSize << "x" << gridSize << endl;
    cout << "Generations: " << generations << endl;
    cout << "Repeats:     " << repeats << endl;
    cout << "========================================" << endl;

    // Output Header
    cout << "\nNeighborhood_Size, Density_Percent, Mean_Time_Seconds" << endl;

    try {
        // Iterate neighborhood sizes 1 to 10
        for (int n = 1; n <= 10; ++n) {

            // Run two tests per neighborhood size: Target Density vs 0.0
            double densitiesToTest[] = { targetDensities[n], 0.0 };

            for (double density : densitiesToTest) {

                // --- 2. DATA PREPARATION ---
                // We must regenerate data every time because density changes
                vector<uint8_t> inputData(gridSize * gridSize);

                for (int i = 0; i < gridSize * gridSize; i++) {
                    if (density == 0.0) {
                        inputData[i] = 0;
                    }
                    else {
                        // Generate random float between 0.0 and 100.0
                        double chance = static_cast<double>(rand()) / RAND_MAX * 100.0;
                        inputData[i] = (chance < density) ? 1 : 0;
                    }
                }

                // --- 3. EXECUTION ---
                double totalTime = 0.0;

                for (int r = 0; r < repeats; r++) {

                    // Instantiate class with current Neighborhood Size (n)
                    // Note: Ensure your GOL_cl class constructor signature is:
                    // GOL_cl(int generations, int gridSize, int radius)
                    GOL_cl_neighborhood sim(generations, gridSize, n);

                    sim.setup_grid(inputData);

                    // Run and accumulate time
                    totalTime += sim.run_simulation();
                }

                double meanTime = totalTime / repeats;

                // Output test results
                cout << "Neighborhood size " << n << " with density " << density << ": " << fixed << setprecision(5) << meanTime << endl;
            }
        }

    }
    catch (const exception& e) {
        cerr << "CRITICAL ERROR: " << e.what() << endl;
        return 1;
    }

    cout << "\nBenchmark Complete." << endl;
    return 0;
}