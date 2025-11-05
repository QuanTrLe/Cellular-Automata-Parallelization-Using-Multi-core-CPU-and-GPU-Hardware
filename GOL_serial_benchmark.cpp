#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace std;

// count neighbor of a given cell
int countNeighbors(const vector<uint8_t>& grid, int r, int c, int rows, int cols) {
    int count = 0;

    for (int dr = -1; dr <= 1; ++dr) { // row offsets from -1 (row above) to 1 (row below)
        for (int dc = -1; dc <= 1; ++dc) { // same but column
            if (dr == 0 && dc == 0) continue; // the current cell we're looking at

            int nr = r + dr, nc = c + dc; // calc that neighbor coords

            // check bounds
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                int n_index = nr * cols + nc; // calc the index we need in the 1D array

                // dont 100% understand but this basically makes it conditional
                // which makes the count++ a variable / volatile work
                // also due to a point in the paper about how variable arithmetic can cause variable speed ups
                if (grid[n_index] == 1) { count++; }
            }
        }
    }
    return count;
}

// print it out for visualization
void printGrid(const vector<uint8_t>& grid, int rows, int cols) {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int index = r * cols + c; // calc the 1D array index
            cout << (grid[index] ? "■ " : ". ");
        }
        cout << '\n';
    }
}

// basically the function that we can just toss the grid in and time it
// passing newGrid in at the start jsut to avoid having to make / allocate stuff every iteration
// also generationLimit is how many tiems we're running the simulation
void run_simulation(vector<uint8_t>& currentGrid, vector<uint8_t>& newGrid, int rows, int cols, int generationLimit) {
    // be zooming through the generations til we're done
    for (int currentGeneration = 0; currentGeneration < generationLimit; ++currentGeneration) {

        // go through all the rows and columns and check + update
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int index = r * cols + c; // calc index
                int n = countNeighbors(currentGrid, r, c, rows, cols); // how many neighbors we have

                if (currentGrid[index]) {
                    newGrid[index] = (n == 2 || n == 3); // if we dead or not
                }
                else {
                    newGrid[index] = (n == 3); // we becomign alive?
                }
            }
        }

        // remember to update to the new grid after checking everything
        currentGrid.swap(newGrid);
    }
}

int main() {
    // size of the grid, as a square currently though
    int rows = 16, cols = 16; // these should be larger but ehhh
    int generationLimit = 10;

    // the array to store the entire lattice / grid in, specifically a 2D array for row and column
    vector<uint8_t> grid(rows * cols); // current grid
    vector<uint8_t> newGrid = grid; // grid after each new generation

    // Currently setting the initial pattern by hand, bit dubious but eh it's jsut testing rn
    // Example: a blinker pattern
    grid[3 * cols + 2] = 1; // (3, 2)
    grid[3 * cols + 3] = 1; // (3, 3)
    grid[3 * cols + 4] = 1; // (3, 4)

    // just outputing so we know
    cout << "Starting Game of Life simulation..." << endl;
    cout << "Grid Size: " << rows << "x" << cols << endl;
    cout << "Generations: " << generationLimit << endl;

    // starting clock
    auto start_time = chrono::high_resolution_clock::now();

    // process the whole thing by passing it into a function
    // i have the power of god and anime on my side
    run_simulation(grid, newGrid, rows, cols, generationLimit);

    // end clock and time
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;

    // end output
    cout << fixed << setprecision(4);
    cout << "\nSimulation finished." << endl;
    cout << "Total serial execution time: " << elapsed.count() << " seconds." << endl;
    cout << "\nFinal Grid State:" << endl;
    printGrid(grid, rows, cols);

    return 0;
}
