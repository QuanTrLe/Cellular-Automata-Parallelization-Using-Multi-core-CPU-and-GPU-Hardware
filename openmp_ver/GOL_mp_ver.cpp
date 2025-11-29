//the real one this time
//compiled with 
//cl /EHsc /openmp GOL_mp_ver.cpp Test_generation_count.cpp (or whatever other script)

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

#include <omp.h>

using namespace std;
class GOL_mp {
private:
    int generationCount;
    int gridSize;
    int paddedSize;
    vector<uint8_t> currentGrid;
    vector<uint8_t> newGrid;

    // count neighbor of a given cell
    inline int countNeighbors(const vector<uint8_t>& grid, int r, int c, int paddedCols) {
        // calc the 1D index of the current cell (r, c)
        // r and c are 1-based (e.g., 1 to 16)
        int idx = r * paddedCols + c;

        // variable arithmetic from the paper without any bounds-checking overhead
        // for cases of border padding, they should be auto 0
        int count =
            grid[idx - paddedCols - 1] +
            grid[idx - paddedCols] +
            grid[idx - paddedCols + 1] +
            grid[idx - 1] +
            grid[idx + 1] +
            grid[idx + paddedCols - 1] +
            grid[idx + paddedCols] +
            grid[idx + paddedCols + 1];

        return count;
    }

public:
    // constructor for the easily passed things
    GOL_mp(int generationCount = 1000, int gridSize = 512) {
        this->generationCount = generationCount;

        this->gridSize = gridSize;
        paddedSize = gridSize + 2;

        // resize vector immediately to match size
        newGrid.resize(paddedSize * paddedSize, 0);
        currentGrid.resize(paddedSize * paddedSize, 0);
    }

    // setup spcifically for passing the initial grid / cell distribution in
    void setup_grid(const vector<uint8_t>& inputGrid) {
        if (inputGrid.size() != gridSize * gridSize) {
            std::cerr << "Error: Input size (" << inputGrid.size() << ") does not match expected grid size (" << gridSize * gridSize << ")!" << std::endl;
            return;
        }

        // copy data from the given grid (unpadded) into the padded grid
        // iterates over row and column to find index and put it in
        for (int r = 0; r < gridSize; r++) {
            for (int c = 0; c < gridSize; c++) {
                int inputIndex = r * gridSize + c;
                int paddedIndex = (r + 1) * paddedSize + (c + 1);

                currentGrid[paddedIndex] = inputGrid[inputIndex];
            }
        }
    }

    // basically the function that we can just toss the grid in and time it
    // passing newGrid in at the start jsut to avoid having to make / allocate stuff every iteration
    // also generationLimit is how many tiems we're running the simulation
    double run_simulation() {

        auto start_time = chrono::high_resolution_clock::now();

        // be zooming through the generations til we're done
        for (int currentGeneration = 0; currentGeneration < this->generationCount; ++currentGeneration) {

            // go through all the rows and columns and check + update
            // remember we're starting at 1 to skip over padded cells
#pragma omp parallel for
            for (int i = 0; i < gridSize * gridSize; ++i) {
                int r = i / gridSize + 1;
                int c = i % gridSize + 1;

                int index = r * paddedSize + c;
                int n = countNeighbors(currentGrid, r, c, paddedSize);

                if (currentGrid[index]) newGrid[index] = (n == 2 || n == 3);
                else newGrid[index] = (n == 3);
            }

            // remember to update to the new grid after checking everything
            currentGrid.swap(newGrid);
        }

        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end_time - start_time;
        return elapsed.count();
    }

    //// print it out for visualization
    //void printGrid(const vector<uint8_t>& grid, int rows, int cols, int paddedCols) {
    //    for (int r = 1; r <= rows; ++r) {
    //        for (int c = 1; c <= cols; ++c) {
    //            int index = r * paddedCols + c; // calc the 1D array index
    //            cout << (grid[index] ? "0 " : ". ");
    //        }
    //        cout << '\n';
    //    }
    //}
};






//int main() {
//    // size of the grid, as a square currently though
//    int rows = 16, cols = 16; // these should be larger but ehhh
//    int paddedRows = rows + 2, paddedCols = cols + 2; // for padding with 0, always padding by 2, 1 on each side
//    int generationLimit = 10;
//    int threads = 4;
//
//    // the array to store the entire lattice / grid in, specifically a 2D array for row and column
//    vector<uint8_t> grid(paddedRows * paddedCols, 0); // current grid
//    vector<uint8_t> newGrid = grid; // grid after each new generation
//
//    // Currently setting the initial pattern by hand, bit dubious but eh it's jsut testing rn
//    // Example: a blinker pattern
//    grid[(3 + 1) * paddedCols + (2 + 1)] = 1; // (3, 2), + 1 becasue of the padding on each side
//    grid[(3 + 1) * paddedCols + (3 + 1)] = 1; // (3, 3)
//    grid[(3 + 1) * paddedCols + (4 + 1)] = 1; // (3, 4)
//
//    // just outputing so we know
//    cout << "Starting Game of Life simulation..." << endl;
//    cout << "Grid Size: " << rows << "x" << cols << endl;
//    cout << "Generations: " << generationLimit << endl;
//
//    // starting clock
//    auto start_time = chrono::high_resolution_clock::now();
//
//    // process the whole thing by passing it into a function
//    // i have the power of god and anime on my side
//    omp_set_num_threads(threads); //also set threads
//    run_simulation(grid, newGrid, rows, cols, generationLimit, paddedCols);
//
//    // end clock and time
//    auto end_time = chrono::high_resolution_clock::now();
//    chrono::duration<double> elapsed = end_time - start_time;
//
//    // end output
//    cout << fixed << setprecision(4);
//    cout << "\nSimulation finished." << endl;
//    cout << "Total OpenMP execution time: " << elapsed.count() << " seconds." << endl;
//    cout << "\nFinal Grid State:" << endl;
//    printGrid(grid, rows, cols, paddedCols);
//
//    return 0;
//}

//int run(int rows, int cols, int generationLimit, int threads) {
//    // size of the grid, as a square currently though
//    int paddedRows = rows + 2, paddedCols = cols + 2; // for padding with 0, always padding by 2, 1 on each side
//
//    // the array to store the entire lattice / grid in, specifically a 2D array for row and column
//    vector<uint8_t> grid(paddedRows * paddedCols, 0); // current grid
//    vector<uint8_t> newGrid = grid; // grid after each new generation
//
//    // Currently setting the initial pattern by hand, bit dubious but eh it's jsut testing rn
//    // Example: a blinker pattern
//    grid[(3 + 1) * paddedCols + (2 + 1)] = 1; // (3, 2), + 1 becasue of the padding on each side
//    grid[(3 + 1) * paddedCols + (3 + 1)] = 1; // (3, 3)
//    grid[(3 + 1) * paddedCols + (4 + 1)] = 1; // (3, 4)
//
//    // just outputing so we know
//    cout << "Starting Game of Life simulation..." << endl;
//    cout << "Grid Size: " << rows << "x" << cols << endl;
//    cout << "Generations: " << generationLimit << endl;
//
//    // starting clock
//    auto start_time = chrono::high_resolution_clock::now();
//
//    // process the whole thing by passing it into a function
//    // i have the power of god and anime on my side
//    omp_set_num_threads(threads); //also set threads
//    run_simulation(grid, newGrid, rows, cols, generationLimit, paddedCols);
//
//    // end clock and time
//    auto end_time = chrono::high_resolution_clock::now();
//    chrono::duration<double> elapsed = end_time - start_time;
//
//    // end output
//    cout << fixed << setprecision(4);
//    cout << "\nSimulation finished." << endl;
//    cout << "Total OpenMP execution time: " << elapsed.count() << " seconds." << endl;
//    cout << "\nFinal Grid State:" << endl;
//    printGrid(grid, rows, cols, paddedCols);
//
//    return 0;
//}