#include <iostream>
#include <vector>
#include <cstdlib> // for rand() and atoi()
#include <ctime>   // for seeding random

#include "..\GOL_cl_ver.cpp"

using namespace std;

int main(int argc, char* argv[]) {
    // ---------------------------------------------------------
    // 1. CONFIGURATION
    // ---------------------------------------------------------
    // Set defaults matching your request
    int generations = 10000;
    int gridSize = 512;
    int workGroupSize = 16;

    // Override defaults if Command Line Arguments are provided
    // Usage: ./program <gens> <size> <workgroup>
    if (argc >= 2) generations = atoi(argv[1]);
    if (argc >= 3) gridSize = atoi(argv[2]);
    if (argc >= 4) workGroupSize = atoi(argv[3]);

    cout << "========================================" << endl;
    cout << "   OpenCL Game of Life - Test Runner    " << endl;
    cout << "========================================" << endl;
    cout << "Generations:    " << generations << endl;
    cout << "Grid Size:      " << gridSize << "x" << gridSize << endl;
    cout << "Workgroup Size: " << workGroupSize << endl;

    // ---------------------------------------------------------
    // 2. DATA PREPARATION
    // ---------------------------------------------------------
    cout << "Initializing random grid data..." << endl;

    // Create a vector of the correct size (unpadded)
    vector<uint8_t> inputData(gridSize * gridSize);

    // Fill with random noise (approx 50% alive)
    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < gridSize * gridSize; i++) {
        inputData[i] = (rand() % 2);
    }

    // ---------------------------------------------------------
    // 3. EXECUTION
    // ---------------------------------------------------------
    try {
        // Instantiate the class
        GOL_cl sim(generations, gridSize, workGroupSize);

        // Load the data (The class handles the padding internally)
        sim.setup_grid(inputData);

        // Run the simulation and capture the time
        cout << "Starting simulation..." << endl;
        double seconds = sim.run_simulation();

        // ---------------------------------------------------------
        // 4. REPORTING
        // ---------------------------------------------------------
        cout << "----------------------------------------" << endl;
        cout << "BENCHMARK RESULT: " << seconds << " seconds" << endl;
        cout << "----------------------------------------" << endl;

    }
    catch (const exception& e) {
        cerr << "CRITICAL ERROR: " << e.what() << endl;
        return 1;
    }

    return 0;
}