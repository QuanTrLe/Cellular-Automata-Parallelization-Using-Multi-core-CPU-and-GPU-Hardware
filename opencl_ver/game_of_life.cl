// the work of a single thread
// in one generation get the neighbor count of a cell and return that state of that cell

int updateGrid(
    __global const char* oldGrid, 
    __global char* newGrid,
    int row, 
    int column, 
    int paddedCols)
{
    int count = 0;

    // calc the 1D index of the current cell (r, c)
    // r and c are 1-based (e.g., 1 to 16)
    int idx = r * paddedCols + c;

    // variable arithmetic from the paper without any bounds-checking overhead
    // for cases of border padding, they should be auto 0
    if (grid[idx - paddedCols - 1]) count++; // top from left to right
    if (grid[idx - paddedCols])     count++;
    if (grid[idx - paddedCols + 1]) count++;

    if (grid[idx - 1])               count++; // mid left and right
    if (grid[idx + 1])               count++;

    if (grid[idx + paddedCols - 1]) count++; // bottom from left to right
    if (grid[idx + paddedCols])     count++;
    if (grid[idx + paddedCols + 1]) count++;

    return count;
}


// basically the function that we can just toss the grid in and time it
// passing newGrid in at the start jsut to avoid having to make / allocate stuff every iteration
// also generationLimit is how many tiems we're running the simulation
void run_simulation(vector<uint8_t>& currentGrid, vector<uint8_t>& newGrid, int rows, int cols, int generationLimit, int paddedCols) {
    // be zooming through the generations til we're done
    for (int currentGeneration = 0; currentGeneration < generationLimit; ++currentGeneration) {

        // go through all the rows and columns and check + update
        // remember we're starting at 1 to skip over padded cells
        for (int r = 1; r <= rows; ++r) {
            for (int c = 1; c <= cols; ++c) {
                int index = r * paddedCols + c; // calc index
                int n = countNeighbors(currentGrid, r, c, paddedCols); // how many neighbors we have

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