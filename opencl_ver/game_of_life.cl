// the work of a single thread
// in one generation get the neighbor count of a cell and return that state of that cell
// arguments not specified are defaulted to __private, also has __constant, __local, and __global
__kernel void updateGrid(
    __global const char* oldGrid, // const because we're only reading from this
    __global char* newGrid, // new grid we'll be writing to
    const int rowSize, // max size for the grid
    const int columnSize,
    const int paddedCols)
{
    int count = 0; // how many neighbors we got
    int row = get_global_id(0);
    int column = get_global_id(1);

    // if row / column is 0 (or smaller than 1) or max then that's in the padded area / not needed
    if (row < 1 || row > rowSize || column < 1 || column > columnSize) {
        return; // eject immediately
    }

    // calc the 1D index of the current cell (row, column)
    int cellIndex = row * paddedCols + column; // row and column are 1-based (1 to 16 or whatever size is)

    // variable arithmetic from the paper without any bounds-checking overhead
    // for cases of border padding, they should be auto 0
    if (oldGrid[cellIndex - paddedCols - 1]) { count++; } // top from left to right
    if (oldGrid[cellIndex - paddedCols]) { count++; }
    if (oldGrid[cellIndex - paddedCols + 1]) { count++; }

    if (oldGrid[cellIndex - 1]) { count++; } // mid left and right
    if (oldGrid[cellIndex + 1]) { count++; }

    if (oldGrid[cellIndex + paddedCols - 1]) { count++; } // bottom from left to right
    if (oldGrid[cellIndex + paddedCols]) { count++; }
    if (oldGrid[cellIndex + paddedCols + 1]) { count++; }

    // if already alive and 2 / 3 neighbor then stay alive ((count == 2 || count == 3) && oldGrid[cellIndex])
    // if currently dead and 3 neighbor then become alive (count == 3 && !oldGrid[cellIndex])
    newGrid[cellIndex] = ( 
        ((count == 2 || count == 3) && oldGrid[cellIndex]) || 
        (count == 3 && !oldGrid[cellIndex])
    );
}


// too long a function?
// if statements?