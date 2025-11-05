#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace std;

// count neighbor of a given cell
int countNeighbors(const vector<vector<uint8_t>>& grid, int r, int c) {
    int rows = grid.size();
    int cols = grid[0].size();
    int count = 0;

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols)
                count += grid[nr][nc];
        }
    }
    return count;
}

// print it out for visualization
void printGrid(const vector<vector<uint8_t>>& grid) {
    for (auto& row : grid) {
        for (auto cell : row)
            cout << (cell ? "■ " : ". ");
        cout << '\n';
    }
}

// basically the function that we can just toss the grid in and time it
// passing newGrid in at the start jsut to avoid having to make / allocate stuff every iteration
// also generationLimit is how many tiems we're running the simulation
void run_simulation(vector<vector<uint8_t>>& currentGrid, vector<vector<uint8_t>>& newGrid, int generationLimit) {

    // getting dimenations of the grid
    int rows = currentGrid.size();
    int cols = currentGrid[0].size();

    // be zooming through the generations til we're done
    for (int currentGeneration = 0; currentGeneration < generationLimit; ++currentGeneration) {

        // go through all the rows and columns and check + update
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int n = countNeighbors(currentGrid, r, c); // how many neighbors we have
                if (currentGrid[r][c]) {
                    newGrid[r][c] = (n == 2 || n == 3); // if we dead or not
                }
                else {
                    newGrid[r][c] = (n == 3); // we becomign alive?
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
    vector<vector<uint8_t>> grid(rows, vector<uint8_t>(cols, 0)); // current grid
    vector<vector<uint8_t>> newGrid = grid; // grid after each new generation

    // Currently setting the initial pattern by hand, bit dubious but eh it's jsut testing rn
    // Example: a blinker pattern
    grid[3][2] = 1;
    grid[3][3] = 1;
    grid[3][4] = 1;

    // just outputing so we know
    cout << "Starting Game of Life simulation..." << endl;
    cout << "Grid Size: " << rows << "x" << cols << endl;
    cout << "Generations: " << generationLimit << endl;

    // starting clock
    auto start_time = chrono::high_resolution_clock::now();

    // process the whole thing by passing it into a function
    // i have the power of god and anime on my side
    run_simulation(grid, newGrid, generationLimit);

    // end clock and time
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;

    // end output
    cout << fixed << setprecision(4);
    cout << "\nSimulation finished." << endl;
    cout << "Total serial execution time: " << elapsed.count() << " seconds." << endl;
    cout << "\nFinal Grid State:" << endl;
    printGrid(grid);

    return 0;
}
