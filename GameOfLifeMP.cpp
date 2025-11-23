#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <omp.h>


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
    omp_set_num_threads(4);
    int rows = 16, cols = 16;
    vector<vector<uint8_t>> grid(rows, vector<uint8_t>(cols, 0));

    // Example: a blinker pattern
    grid[3][2] = 1;
    grid[3][3] = 1;
    grid[3][4] = 1;

    while (true) {
        system("cls");
        printGrid(grid);

        vector<vector<uint8_t>> newGrid = grid;

        #pragma omp parallel for
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
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    return 0;
}
