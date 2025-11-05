#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

using namespace std;

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

void printGrid(const vector<vector<uint8_t>>& grid) {
    for (auto& row : grid) {
        for (auto cell : row)
            cout << (cell ? "■ " : ". ");
        cout << '\n';
    }
}

int main() {
    // size of the grid, as a square currently though
    int rows = 16, cols = 16;

    // the array to store the entire lattice / grid in, specifically a 2D array for row and column
    vector<vector<uint8_t>> grid(rows, vector<uint8_t>(cols, 0));

    // Currently setting the initial pattern by hand, bit dubious but eh it's jsut testing rn
    // Example: a blinker pattern
    grid[3][2] = 1;
    grid[3][3] = 1;
    grid[3][4] = 1;

    while (true) {
        system("cls"); // since we're printing the thing out everytime, clear for sake of mind
        printGrid(grid); // print new grid out

        vector<vector<uint8_t>> newGrid = grid; // make sure we update the grid we're using

        // go through all the rows and columns
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int n = countNeighbors(grid, r, c);
                if (grid[r][c]) {
                    newGrid[r][c] = (n == 2 || n == 3);
                }
                else {
                    newGrid[r][c] = (n == 3);
                }
            }
        }

        grid.swap(newGrid);
        this_thread::sleep_for(chrono::milliseconds(1000)); // wait before we calc and print new grid
    }

    return 0;
}
