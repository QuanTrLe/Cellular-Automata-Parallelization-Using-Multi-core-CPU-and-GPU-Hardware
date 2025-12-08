//the real one this time
//compiled with 
// cl /EHsc /openmp GOL_mp_neighbors.cpp Testing/Test_neighborhood.cpp

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

#include <omp.h>

using namespace std;
class GOL_mp_neighborhood {
private:
    int generationCount;
    int gridSize;
    int paddedSize;
    int neighborRadius;

    int surviveLowerThreshold; // 2/8
    int surviveUpperThreshold; // 3/8
    vector<uint8_t> currentGrid;
    vector<uint8_t> newGrid;

    // count neighbor of a given cell
    inline int countNeighbors(const vector<uint8_t>& grid, int r, int c, int paddedCols, int radius) {
        // calc the 1D index of the current cell (r, c)
        // r and c are 1-based (e.g., 1 to 16)
        int idx = r * paddedCols + c;

        // variable arithmetic from the paper without any bounds-checking overhead
        // for cases of border padding, they should be auto 0
        int count = 0;

        for (int dr = -radius; dr <= radius; ++dr) {
            int base = idx + dr * paddedCols;
            for (int dc = -radius; dc <= radius; ++dc) {
                if (dr == 0 && dc == 0) continue; // skip center
                count += grid[base + dc];
            }
        }
        return count;
    }

public:
    // constructor for the easily passed things
    GOL_mp_neighborhood(int generationCount = 1000, int gridSize = 512, int neighborRadius = 1) {
        this->generationCount = generationCount;
        this->gridSize = gridSize;
        this->neighborRadius = neighborRadius;

        int neighbors = pow(2 * (neighborRadius)+1, 2) - 1;

        this->surviveLowerThreshold = floor(neighbors * (2.0 / 8.0));
        this->surviveUpperThreshold = ceil(neighbors * (3.0 / 8.0));

        paddedSize = gridSize + 2 * neighborRadius;

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
                int paddedIndex = (r + neighborRadius) * paddedSize + (c + neighborRadius);

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
                int r = i / gridSize + neighborRadius;
                int c = i % gridSize + neighborRadius;

                int index = r * paddedSize + c;
                int n = countNeighbors(currentGrid, r, c, paddedSize, neighborRadius);

                if (currentGrid[index]) {
                    // alive cell survives if neighbors in [lower, upper]
                    newGrid[index] = (n >= surviveLowerThreshold && n <= surviveUpperThreshold);
                }
                else {
                    // dead cell is born if neighbors == upper threshold
                    newGrid[index] = (n == surviveUpperThreshold);
                }
            }

            // remember to update to the new grid after checking everything
            currentGrid.swap(newGrid);
        }

        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end_time - start_time;
        return elapsed.count();
    }
};