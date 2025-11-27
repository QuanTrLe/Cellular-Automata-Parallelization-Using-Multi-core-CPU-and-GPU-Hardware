// the work of a single thread
// in one generation get the neighbor count of a cell and return that state of that cell
// arguments not specified are defaulted to __private, also has __constant, __local, and __global
__kernel void updateGrid(
    __global const char* oldGrid,       // const because we're only reading from this
    __global char* newGrid,             // new grid we'll be writing to
    const int rowSize,                  // max size for the grid, need for special cases
    const int columnSize, 
    const int paddedColumns)
{
    int count = 0; // how many neighbors we got

    // getting row and column by using global_id of the thread
    // originally was gonna do id 0 row and 1 column, but swapped due to memory coalescence
    int row = get_global_id(1);
    int column = get_global_id(0);

    // if row / column is 0 (or smaller than 1) or max then that's in the padded area / not needed
    if (row < 1 || row > rowSize || column < 1 || column > columnSize) {
        return; // eject immediately
    }

    // calc the 1D index of the current cell (row, column)
    int cellIndex = row * paddedColumns + column; // row and column are 1-based (1 to 16 or whatever size is)

    // variable arithmetic from the paper without any bounds-checking overhead
    // for cases of border padding, they should be auto 0
    count += oldGrid[cellIndex - paddedColumns - 1]; // top from left to right
    count += oldGrid[cellIndex - paddedColumns];
    count += oldGrid[cellIndex - paddedColumns + 1];

    count += oldGrid[cellIndex - 1]; // mid left and right
    count += oldGrid[cellIndex + 1];

    count += oldGrid[cellIndex + paddedColumns - 1]; // bottom from left to right
    count += oldGrid[cellIndex + paddedColumns];
    count += oldGrid[cellIndex + paddedColumns + 1];

    // if neighbor count = 3 then alive, no matter previous state
    // but if 2 then only alive if was currently alive
    newGrid[cellIndex] = (count == 3) | (count == 2 && oldGrid[cellIndex]);
}