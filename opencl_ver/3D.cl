// the work of a single thread
// in one generation get the neighbor count of a cell and return that state of that cell
// arguments not specified are defaulted to __private, also has __constant, __local, and __global
__kernel void updateGrid(
    __global const char* oldGrid,       // const because we're only reading from this
    __global char* newGrid,             // new grid we'll be writing to
    const int dimX,          // columns
    const int dimY,          // rows
    const int dimZ,          // depth
    const int paddedSize)
{
    int count = 0; // how many neighbors we got

    // getting row and column by using global_id of the thread
    // originally was gonna do id 0 row and 1 column, but swapped due to memory coalescence
    int x = get_global_id(0);
    int y = get_global_id(1);
    int z = get_global_id(2);

    // if row / column is 0 (or smaller than 1) or max then that's in the padded area / not needed
    if (x < 1 || x > dimX || y < 1 || y > dimY || z < 1 || z > dimZ)
        return;

    int slice = paddedSize * paddedSize;
    int rowStride = paddedSize;
    int cellIndex = z * slice + y * rowStride + x;

    // iterate over neighbors in 3D
    // Top layer (z-1)
    count += oldGrid[cellIndex - slice - rowStride - 1];
    count += oldGrid[cellIndex - slice - rowStride];
    count += oldGrid[cellIndex - slice - rowStride + 1];
    count += oldGrid[cellIndex - slice - 1];
    count += oldGrid[cellIndex - slice];
    count += oldGrid[cellIndex - slice + 1];
    count += oldGrid[cellIndex - slice + rowStride - 1];
    count += oldGrid[cellIndex - slice + rowStride];
    count += oldGrid[cellIndex - slice + rowStride + 1];

    // Middle layer (z)
    count += oldGrid[cellIndex - rowStride - 1];
    count += oldGrid[cellIndex - rowStride];
    count += oldGrid[cellIndex - rowStride + 1];
    count += oldGrid[cellIndex - 1];
    count += oldGrid[cellIndex + 1];
    count += oldGrid[cellIndex + rowStride - 1];
    count += oldGrid[cellIndex + rowStride];
    count += oldGrid[cellIndex + rowStride + 1];

    // Bottom layer (z+1)
    count += oldGrid[cellIndex + slice - rowStride - 1];
    count += oldGrid[cellIndex + slice - rowStride];
    count += oldGrid[cellIndex + slice - rowStride + 1];
    count += oldGrid[cellIndex + slice - 1];
    count += oldGrid[cellIndex + slice];
    count += oldGrid[cellIndex + slice + 1];
    count += oldGrid[cellIndex + slice + rowStride - 1];
    count += oldGrid[cellIndex + slice + rowStride];
    count += oldGrid[cellIndex + slice + rowStride + 1];



    // if neighbor count = 3 then alive, no matter previous state
    // but if 2 then only alive if was currently alive
    newGrid[cellIndex] = (count == 3) || (count == 2 && oldGrid[cellIndex]);
}