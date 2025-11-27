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
void checkError(cl_int err, const char* operationName) {
	if (err != CL_SUCCESS) {
		cerr << "Error during " << operationName << ": " << err << endl;
		exit(1);
	}
}


// for testing / debugging
// print it out for visualization
void printGrid(const vector<uint8_t>& grid, int rows, int cols, int paddedCols) {
	for (int r = 1; r <= rows; ++r) {
		for (int c = 1; c <= cols; ++c) {
			int index = r * paddedCols + c; // calc the 1D array index
			cout << (grid[index] ? "■ " : ". ");
		}
		cout << '\n';
	}
}


int main(int argc, char* argv[]) {
	// start time randomly just putting here
	// making sure we overshoot the time rather than leaving out actual time
	auto start_time = chrono::high_resolution_clock::now();

	// details of grid and running
	int rows = 1024, columns = 1024;
	int paddedRows = rows + 2, paddedColumns = columns + 2;

	int generationLimit = 10000;

	size_t globalWorkGroupSize[2];
	size_t localWorkGroupSize[2];
	cl_int err; // for reporting error codes
	
	// for the below we can jsut pass NULL to let it auto decide
	localWorkGroupSize[0] = 16; // # of work items in each local work group
	localWorkGroupSize[1] = 16;
	globalWorkGroupSize[0] = (size_t) ceil(columns / (float)localWorkGroupSize[0]) * localWorkGroupSize[0]; // # of total work items, localSize MUST be devisor
	globalWorkGroupSize[1] = (size_t) ceil(rows / (float)localWorkGroupSize[1]) * localWorkGroupSize[1];


	// initializing the grid as a 1D array of chars + blinker pattern
	vector<uint8_t> grid(paddedRows * paddedColumns, 0); // current grid
	vector<uint8_t> newGrid = grid; // grid after each new generation
	// blinker pattern
	int startRow = 3 + 1, startCol = 2 + 1;

	grid[startRow * paddedColumns + startCol] = 1;       // top
	grid[startRow * paddedColumns + (startCol + 1)] = 1; // middle
	grid[startRow * paddedColumns + (startCol + 2)] = 1; // bottom


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
	err |= clSetKernelArg(kernel, 2, sizeof(int), &rows); // setting const args for the kernel func
	err |= clSetKernelArg(kernel, 3, sizeof(int), &columns);
	err |= clSetKernelArg(kernel, 4, sizeof(int), &paddedColumns);


	// executing kernel over entire range of data set
	// go over til we hti the generation limit, ping pong the grid we're writing on as newGrid
	cout << "Running " << generationLimit << " generations..." << endl;

	for (int i = 0; i < generationLimit; i++) {
		// setting the grid arguments for each generation, changing what grid we write to and what we use as base grid
		err = clSetKernelArg(kernel, i%2, sizeof(cl_mem), &d_gridA);
		err |= clSetKernelArg(kernel, (i+1)%2, sizeof(cl_mem), &d_gridB);
		checkError(err, "Setting Kernel Args");

		err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalWorkGroupSize, localWorkGroupSize, 0, NULL, NULL);
		checkError(err, "EnqueueNDRangeKernel");
	}


	// waiting for command queue to get servuiced before reading back results
	clFinish(queue);

	// read said results and print it out to test
	cl_mem finalResultBuffer = (generationLimit % 2 == 0) ? d_gridA : d_gridB; // which grid to read from
	clEnqueueReadBuffer(queue, finalResultBuffer, CL_TRUE, 0, dataSize, grid.data(), 0, NULL, NULL);

	// disabled these bc grid too big to print out and be useful anyway
	// cout << "\nFinal Grid State (GPU Result):" << endl;
	// printGrid(grid, rows, columns, paddedColumns);
	// end clock and time
	auto end_time = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = end_time - start_time;
	cout << "Total serial execution time: " << elapsed.count() << " seconds." << endl;


	// free all the resources we held
	clReleaseMemObject(d_gridA);
	clReleaseMemObject(d_gridB);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return 0;
}
