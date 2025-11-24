#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include<fstream>
#include<sstream>
#include <CL/cl.h>

#define PROGRAM_FILE "game_of_life.cl"
#define KERNEL_FUNC "updateGrid"

using namespace std;

//// basically the function that we can just toss the grid in and time it
//// passing newGrid in at the start jsut to avoid having to make / allocate stuff every iteration
//// also generationLimit is how many tiems we're running the simulation
//void run_simulation(vector<uint8_t>& currentGrid, vector<uint8_t>& newGrid, int rows, int cols, int generationLimit, int paddedCols) {
//    // be zooming through the generations til we're done
//    for (int currentGeneration = 0; currentGeneration < generationLimit; ++currentGeneration) {
//
//        // go through all the rows and columns and check + update
//        // remember we're starting at 1 to skip over padded cells
//        for (int r = 1; r <= rows; ++r) {
//            for (int c = 1; c <= cols; ++c) {
//                int index = r * paddedCols + c; // calc index
//                int n = countNeighbors(currentGrid, r, c, paddedCols); // how many neighbors we have
//
//                if (currentGrid[index]) {
//                    newGrid[index] = (n == 2 || n == 3); // if we dead or not
//                }
//                else {
//                    newGrid[index] = (n == 3); // we becomign alive?
//                }
//            }
//        }
//
//        // remember to update to the new grid after checking everything
//        currentGrid.swap(newGrid);
//    }
//}
//
//int test() {
//    // size of the grid, as a square currently though
//    int rows = 16, cols = 16; // these should be larger but ehhh
//    int paddedRows = rows + 2, paddedCols = cols + 2; // for padding with 0, always padding by 2, 1 on each side
//    int generationLimit = 10;
//
//    // the array to store the entire lattice / grid in, specifically a 2D array for row and column
//    vector<uint8_t> grid(paddedRows * paddedCols, 0); // current grid
//    vector<uint8_t> newGrid = grid; // grid after each new generation
//
//    // Currently setting the initial pattern by hand, bit dubious but eh it's jsut testing rn
//    // Example: a blinker pattern
//    grid[(3 + 1) * paddedCols + (2 + 1)] = 1; // (3, 2), + 1 becasue of the padding on each side
//    grid[(3 + 1) * paddedCols + (3 + 1)] = 1; // (3, 3)
//    grid[(3 + 1) * paddedCols + (4 + 1)] = 1; // (3, 4)
//
//    // just outputing so we know
//    cout << "Starting Game of Life simulation..." << endl;
//    cout << "Grid Size: " << rows << "x" << cols << endl;
//    cout << "Generations: " << generationLimit << endl;
//
//    // starting clock
//    auto start_time = chrono::high_resolution_clock::now();
//
//    // process the whole thing by passing it into a function
//    // i have the power of god and anime on my side
//    run_simulation(grid, newGrid, rows, cols, generationLimit, paddedCols);
//
//    // end clock and time
//    auto end_time = chrono::high_resolution_clock::now();
//    chrono::duration<double> elapsed = end_time - start_time;
//
//    // end output
//    cout << fixed << setprecision(4);
//    cout << "\nSimulation finished." << endl;
//    cout << "Total serial execution time: " << elapsed.count() << " seconds." << endl;
//    cout << "\nFinal Grid State:" << endl;
//    printGrid(grid, rows, cols, paddedCols);
//
//    return 0;
//}

// helper function to read the .cl file into a string
// needed for creating program with source for OpenCL
string loadKernelSource(const char* filename) {
	ifstream file(filename);
	if (!file.is_open()) { // jsut making sure we aok
		cerr << "Error: Could not open kernel file: " << filename << endl;
		exit(1);
	}

	stringstream buffer; // get the actual thing into buffer
	buffer << file.rdbuf();
	return buffer.str(); // then return as string
}


// for checking the build log / error returned by the OpenCL functions / operations
void checkBuildError(cl_int err, cl_program program, cl_device_id device_id) {
	if (err != CL_SUCCESS) {
		// get the size of log
		size_t len;
		char buffer[2048];

		// ask OpenCL for log text
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);

		// print it and crash afterwards lmao
		cout << "--- Build Log --- \n" << buffer << endl;
		exit(1);
	}
}


int main(int argc, char* argv[]) {
	// details of grid and running
	int rows = 16, columns = 16;
	int paddedRows = rows + 2, paddedColumns = columns + 2;

	int generationLimit = 10;

	size_t globalWorkGroupSize[2];
	size_t localWorkGroupSize[2];
	cl_int err; // for reporting error codes
	
	// for the below we can jsut pass NULL to let it auto decide
	localWorkGroupSize[0] = 8; // # of work items in each local work group
	localWorkGroupSize[1] = 8;
	globalWorkGroupSize[0] = (size_t) ceil(columns / (float)localWorkGroupSize[0]) * localWorkGroupSize[0]; // # of total work items, localSize MUST be devisor
	globalWorkGroupSize[1] = (size_t) ceil(rows / (float)localWorkGroupSize[1]) * localWorkGroupSize[1];


	// initializing the grid as a 1D array of chars
	vector<uint8_t> grid(paddedRows * paddedColumns, 0); // current grid
	vector<uint8_t> newGrid = grid; // grid after each new generation

	// device / kernel input buffers
	cl_mem d_gridA; // d_ is jsut for marking as for device
	cl_mem d_gridB; // these two will be changing as old and new later
	size_t dataSize = paddedRows * paddedColumns * sizeof(uint8_t); // how big we making these


	// OpenCL information
	cl_platform_id cpPlatform;        // OpenCL platform
	cl_device_id device_id;           // device ID
	cl_context context;               // context
	cl_command_queue queue;           // command queue
	cl_program program;               // program
	cl_kernel kernel;                 // kernel

	// getting the info for the OpenCL stuffs: platforms, devices, contexes
	// bind to platform + ID for device
    err = clGetPlatformIDs(1, &cpPlatform, NULL);
	err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL); // specifically using GPU

	// creating context and command queue
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	queue = clCreateCommandQueue(context, device_id, 0, &err);

	// compute the program fro mthe source buffer and building the program executable
	string sourceString = loadKernelSource(PROGRAM_FILE); // have to get the thing as str
	const char* sourcePointer = sourceString.c_str();
	size_t sourceSize = sourceString.length();

	program = clCreateProgramWithSource(context, 1, &sourcePointer, &sourceSize, &err);
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL); // passing 0 is telling compiling for every device in context
	checkBuildError(err, program, device_id); // checking we ok

	// creating kernel in the program to be run
	kernel = clCreateKernel(program, "updateGrid", &err);


	// creating input and output arrays in device memory for calculation
	d_gridA = clCreateBuffer(context, CL_MEM_READ_WRITE, dataSize, NULL, NULL);
	d_gridB = clCreateBuffer(context, CL_MEM_READ_WRITE, dataSize, NULL, NULL);

	// write data set into input array + setting arguments to compute kernel
	// not giving one to gridB bc that's the output first gen
	err = clEnqueueWriteBuffer(queue, d_gridA, CL_TRUE, 0, dataSize, grid.data(), 0, NULL, NULL);
	err |= clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_gridA); // setting args for the kernel func
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_gridB);
	err |= clSetKernelArg(kernel, 2, sizeof(int), &rows);
	err |= clSetKernelArg(kernel, 3, sizeof(int), &columns);
	err |= clSetKernelArg(kernel, 4, sizeof(int), &paddedColumns);


	// executing kernel over entire range of data set
	// go over til we hti the generation limit
	err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalWorkGroupSize, localWorkGroupSize, 0, NULL, NULL);

	// waiting for command queue to get servuiced before reading back results
	clFinish(queue);
}
