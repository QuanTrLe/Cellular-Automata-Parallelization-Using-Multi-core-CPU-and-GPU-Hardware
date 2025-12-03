#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace std;

class GOL_serial {
private:
    int generationCount;
    int gridSize;
    int paddedSize;
    vector<uint8_t> currentGrid;
    vector<uint8_t> newGrid;

    // Count neighbors of a given cell
    // Inline for performance in serial execution
    inline int countNeighbors(const vector<uint8_t>& grid, int r, int c, int paddedCols) {
        int idx = r * paddedCols + c;

        // Summing neighbors
        int count = grid[idx - paddedCols - 1] + grid[idx - paddedCols] + grid[idx - paddedCols + 1]; // top left -> right

        count += grid[idx - 1] + grid[idx + 1]; // left and right of middle row

        count += grid[idx + paddedCols - 1] + grid[idx + paddedCols] + grid[idx + paddedCols + 1]; // bottom left -> right

        return count;
    }

public:
    // Constructor matching the MP version
    GOL_serial(int generationCount = 1000, int gridSize = 512) {
        this->generationCount = generationCount;
        this->gridSize = gridSize;
        this->paddedSize = gridSize + 2;

        // Resize vectors immediately
        // Initialize with 0
        newGrid.resize(paddedSize * paddedSize, 0);
        currentGrid.resize(paddedSize * paddedSize, 0);
    }

    // Setup specific for passing the initial grid / cell distribution in
    // Exact copy of the MP version logic to ensure fair benchmarking
    void setup_grid(const vector<uint8_t>& inputGrid) {
        if (inputGrid.size() != gridSize * gridSize) {
            std::cerr << "Error: Input size (" << inputGrid.size() << ") does not match expected grid size (" << gridSize * gridSize << ")!" << std::endl;
            return;
        }

        // Copy data from the given grid (unpadded) into the padded grid
        for (int r = 0; r < gridSize; r++) {
            for (int c = 0; c < gridSize; c++) {
                int inputIndex = r * gridSize + c;
                int paddedIndex = (r + 1) * paddedSize + (c + 1);

                currentGrid[paddedIndex] = inputGrid[inputIndex];
            }
        }
    }

    // Run simulation and return elapsed time
    double run_simulation() {
        auto start_time = chrono::high_resolution_clock::now();

        for (int currentGeneration = 0; currentGeneration < this->generationCount; ++currentGeneration) {

            // Serial nested loops (standard row-major traversal)
            // Starting at 1 to skip padding
            for (int r = 1; r <= gridSize; ++r) {
                for (int c = 1; c <= gridSize; ++c) {

                    int index = r * paddedSize + c;
                    int n = countNeighbors(currentGrid, r, c, paddedSize);

                    // Apply Game of Life rules:
                    // Alive (1) if neighbors == 3 OR (neighbors == 2 AND currently alive)
                    newGrid[index] = (n == 3) | (n == 2 && currentGrid[index]);
                }
            }

            // Swap grids after processing the whole generation
            currentGrid.swap(newGrid);
        }

        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end_time - start_time;

        return elapsed.count();
    }
};