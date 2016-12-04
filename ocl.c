/*
* Copyright 2016 sprocket
* Copyright 2016 Evil-Knievel
*
* This program is free software; you can redistribuSte it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*/

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define _GNU_SOURCE

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>

#include "miner.h"

#define CL_CHECK(x) x

int ocl_cores = 0;
size_t dimensions = 0;
size_t* sizes;

// OpenCL Buffers
cl_mem base_data;
cl_mem input;
cl_mem output;

// OpenCL State
cl_command_queue queue;

// OpenCL Kernels
cl_kernel kernel_initialize;
cl_kernel kernel_execute;

cl_device_id device_id;
cl_context context;

extern unsigned char* load_opencl_source(char *work_str) {
	unsigned char filename[50], *ocl_source = NULL;
	FILE *fp;
	size_t bytes;

	if (!work_str || (strlen(work_str) > 22)) {
		applog(LOG_ERR, "ERROR: Invalid filename for OpenCL source: %s", work_str);
		return NULL;
	}

	sprintf(filename, "./work/%s.cl", work_str);

	// Load The Source Code For The OpenCL Kernels
	fp = fopen(filename, "r");
	if (!fp) {
		applog(LOG_ERR, "ERROR: Failed to load OpenCL source: %s", filename);
		return NULL;
	}

	ocl_source = (char*)malloc(MAX_SOURCE_SIZE);
	bytes = fread(ocl_source, 1, MAX_SOURCE_SIZE, fp);
	ocl_source[bytes] = 0;	// Terminating Zero
	fclose(fp);

	if (bytes <= 0) {
		applog(LOG_ERR, "ERROR: Failed to read OpenCL source: %s", filename);
		free(ocl_source);
		return NULL;
	}

	return ocl_source;
}

extern bool prepare_opencl_kernels(char *ocl_source) {
	cl_int err;

	// Create Kernels
	kernel_initialize = create_opencl_kernel(device_id, context, ocl_source, "initialize");
	kernel_execute = create_opencl_kernel(device_id, context, ocl_source, "execute");

	// Create Buffers
	base_data = clCreateBuffer(context, CL_MEM_READ_ONLY, 96 * sizeof(char), NULL, &err);


	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "Unable to create OpenCL buffers: base_data / %d", err);
		return false;
	}

	input = clCreateBuffer(context, CL_MEM_READ_WRITE, ocl_cores * VM_MEMORY_SIZE * sizeof(int32_t), NULL, &err);

	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "Unable to create OpenCL buffers: input / %d", err);
		return false;
	}

	output = clCreateBuffer(context, CL_MEM_READ_WRITE, ocl_cores * sizeof(uint32_t), NULL, &err);

	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "Unable to create OpenCL buffers: output / %d", err);
		return false;
	}

	// Set Argurments For Kernels
	err = clSetKernelArg(kernel_initialize, 0, sizeof(cl_mem), (const void*)&input);
	err |= clSetKernelArg(kernel_initialize, 1, sizeof(cl_mem), (const void*)&base_data);
	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "Unable to set OpenCL argurments for 'initialize'");
		return false;
	}

	err = clSetKernelArg(kernel_execute, 0, sizeof(cl_mem), (const void*)&input);
	err |= clSetKernelArg(kernel_execute, 1, sizeof(cl_mem), (const void*)&output);
	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "Unable to set OpenCL argurments for 'execute'");
		return false;
	}

	return true;
}

extern cl_kernel create_opencl_kernel(cl_device_id device_id, cl_context context, const char *source, const char *name) {
	int err;

	// Load OpenCL Source Code
	cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &err);
	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "Unable to load OpenCL program");
		return NULL;
	}

	// Compile OpenCL Source Code
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		size_t len;
		char buffer[2048];
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		applog(LOG_ERR, "Unable to compile OpenCL program -\n%s", buffer);
	}

	// Create OpenCL Kernel
	cl_kernel kernel = clCreateKernel(program, name, &err);
	if (!kernel || err != CL_SUCCESS) {
		applog(LOG_ERR, "Unable to create OpenCL kernel");
		clReleaseProgram(program);
		return NULL;
	}

	return kernel;
}

extern bool initialize_opencl() {
	int i, j;

	cl_platform_id platforms[100];
	cl_uint err;
	cl_uint platforms_n = 0;
	cl_uint devices_n = 0;
	uint32_t global_mem;
	size_t compute_units = 0;
	size_t max_work_size = 0;
	int max_cores = 0;
	char buffer[10240];

	CL_CHECK(clGetPlatformIDs(100, platforms, &platforms_n));

	applog(LOG_DEBUG, "=== %d OpenCL platform(s) found: ===", platforms_n);
	for (i = 0; i < platforms_n; i++) {
		applog(LOG_DEBUG, "  -- %d --", i);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, sizeof(buffer), buffer, NULL));
		applog(LOG_DEBUG, "  PROFILE = %s", buffer);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(buffer), buffer, NULL));
		applog(LOG_DEBUG, "  VERSION = %s", buffer);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL));
		applog(LOG_DEBUG, "  NAME = %s", buffer);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buffer), buffer, NULL));
		applog(LOG_DEBUG, "  VENDOR = %s", buffer);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, sizeof(buffer), buffer, NULL));
		applog(LOG_DEBUG, "  EXTENSIONS = %s", buffer);

		err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &devices_n);
		if (err != CL_SUCCESS) {
			applog(LOG_ERR, "Error: Unable to get devices from OpenCL Platform %d (Error: %d)", i, err);
			return false;
		}

		if (devices_n) {
			applog(LOG_DEBUG, "  DEVICES:");
			cl_device_id *devices = (cl_device_id *)malloc(devices_n * sizeof(cl_device_id));

			clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, devices_n, devices, NULL);
			for (j = 0; j < devices_n; j++) {
				clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);
				applog(LOG_DEBUG, "    %d - %s", j, buffer);
			}
			free(devices);
		}
	}

	if (platforms_n == 0) {
		applog(LOG_ERR, "No OpenCL platforms found");
		return false;
	}

	err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 1, &device_id, NULL);
	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "Unable to enumerate OpenCL device IDs");
		return false;
	}

	clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(size_t), &global_mem, NULL);
	applog(LOG_DEBUG, "  CL_DEVICE_GLOBAL_MEM_SIZE = %lu", global_mem);

	clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &dimensions, NULL);
	applog(LOG_DEBUG, "  CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS  = %zu", dimensions);

	clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_work_size, NULL);
	applog(LOG_DEBUG, "  CL_DEVICE_MAX_WORK_GROUP_SIZE  = %zu", max_work_size);

	clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(size_t), &compute_units, NULL);
	applog(LOG_DEBUG, "  CL_DEVICE_MAX_COMPUTE_UNITS  = %zu", compute_units);



	sizes = (size_t*)malloc(sizeof(size_t)*dimensions);

	clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*dimensions, sizes, NULL);
	for (i = 0; i<dimensions; ++i)
		applog(LOG_DEBUG, "  CL_DEVICE_MAX_WORK_ITEM_SIZES dim %d = %zu", i, sizes[i]);

	// Calculate optimal core number
	// GLOB MEM NEEDED: 96 + (x * VM_MEMORY_SIZE * sizeof(int32_t)) + (x * sizeof(uint32_t))
	double calc = ((double)global_mem - 96.0 - 650 * 1024 * 1024 /*Some 650 M space for who knows what*/) / ((double)VM_MEMORY_SIZE * sizeof(int32_t) + sizeof(int32_t));
	size_t bound = (size_t)calc;

	max_cores = (compute_units - 1) * 64; // Max 64 Shaders Per Compute Unit
	ocl_cores = (bound < max_cores) ? (int)bound : max_cores;
	applog(LOG_INFO, "Global GPU Memory = %lu, Using %d Cores", global_mem, ocl_cores);

	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if (!context) {
		applog(LOG_ERR, "Unable to create OpenCL context");
		return false;
	}

	queue = clCreateCommandQueue(context, device_id, 0, &err);
	if (!queue) {
		applog(LOG_ERR, "Unable to create OpenCL command queue: %d", err);
		return false;
	}

	return true;
}

extern bool execute_kernel(const uint32_t *vm_input, uint32_t *vm_out) {
	int err;

	// Copy Random Inputs To OpenCL Buffer
	err = clEnqueueWriteBuffer(queue, base_data, CL_TRUE, 0, 96 * sizeof(char), vm_input, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "ERROR: Unable to write to OpenCL 'base_data' Buffer");
		return false;
	}

	size_t first_dim = (ocl_cores > sizes[0]) ? sizes[0] : ocl_cores;
	size_t second_dim = 1;
	if (dimensions >= 2) {
		second_dim = (size_t)(ocl_cores>sizes[0]) ? (ceil((double)ocl_cores / (double)sizes[0])) : 1;
	}

	const size_t global[2] = { first_dim, second_dim };
	const size_t local[2] = { 128, 64 };

	//printf("Dimensions: (ocl cores %d, dims %d) %zu %zu\n",ocl_cores,dimensions,global[0],global[1]);

	// Initialize OpenCL VM Data w/ Random Inputs
	err = clEnqueueNDRangeKernel(queue, kernel_initialize, 1, NULL, &global[0], &local[0], 0, NULL, NULL);
	if (err) {
		applog(LOG_ERR, "ERROR: Unable to run 'initialize' kernel: %d", err);
		return false;
	}

	// Run OpenCL VM
	err = clEnqueueNDRangeKernel(queue, kernel_execute, 1, NULL, &global[0], &local[0], 0, NULL, NULL);
	if (err) {
		applog(LOG_ERR, "ERROR: Unable to run 'execute' kernel: %d", err);
		return false;
	}

	// Get VM Output
	err = clEnqueueReadBuffer(queue, output, CL_TRUE, 0, ocl_cores * sizeof(uint32_t), vm_out, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "ERROR: Unable to read OpenCL 'output' Buffer: %d", err);
		return false;
	}

	return true;
}
