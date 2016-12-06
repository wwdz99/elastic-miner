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

extern bool init_opencl_kernel(int id, char *ocl_source) {
	cl_int err;

	// Create Kernel
	gpu[id].kernel_execute = create_opencl_kernel(gpu[id].device_id, gpu[id].context, ocl_source, "execute");

	// Set Argurments For Kernel
	err  = clSetKernelArg(gpu[id].kernel_execute, 0, sizeof(cl_mem), (const void*)&gpu[id].vm_input);
	err |= clSetKernelArg(gpu[id].kernel_execute, 1, sizeof(cl_mem), (const void*)&gpu[id].vm_mem);
	err |= clSetKernelArg(gpu[id].kernel_execute, 2, sizeof(cl_mem), (const void*)&gpu[id].vm_out);

	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "Unable to set OpenCL argurments for 'execute' kernel (Error: %d)", err);
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

extern int init_opencl_devices() {
	int i, j, k;
	cl_platform_id platforms[100];
	cl_uint err;
	cl_uint platforms_n = 0;
	cl_uint devices_n = 0;
	uint32_t global_mem;
	size_t compute_units = 0;
	size_t max_work_size = 0;
	size_t dimensions = 0;
	int max_cores = 0;
	char buffer[256];
	int num_devices = 0;
	bool found;
	int gpu_cnt = 0;

	CL_CHECK(clGetPlatformIDs(100, platforms, &platforms_n));

	if (platforms_n == 0) {
		applog(LOG_ERR, "ERROR: No OpenCL platforms found!");
		return 0;
	}

	applog(LOG_DEBUG, "=== %d OpenCL platform(s) found: ===", platforms_n);

	gpu = (struct opencl_device *)malloc(platforms_n * sizeof(struct opencl_device));

	if (!gpu) {
		applog(LOG_ERR, "ERROR: Unable to allocate GPU devices!");
		return 0;
	}

	for (i = 0; i < platforms_n; i++) {

		found = false;

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
			return 0;
		}

		if (devices_n) {

			applog(LOG_DEBUG, "  DEVICES:");
			cl_device_id *devices = (cl_device_id *)malloc(devices_n * sizeof(cl_device_id));

			err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, devices_n, devices, NULL);

			if (err != CL_SUCCESS)
				devices_n = 0;

			for (j = 0; j < devices_n; j++) {

				err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);

				if (err != CL_SUCCESS)
					break;

				applog(LOG_DEBUG, "    %d - %s", j, buffer);

				// Convert Device Name To Lowercase
				for (k = 0; k < strlen(buffer); k++)
					buffer[k] = toupper(buffer[k]);

				// Only Count NVDIA, AMD and Built In Intel Graphics
				if (strstr(buffer, "NVIDIA") || strstr(buffer, "AMD") || strstr(buffer, "GRAPHICS")) {
					memcpy(&gpu[gpu_cnt].platform_id, &platforms[i], sizeof(cl_platform_id));
					memcpy(&gpu[gpu_cnt].device_id, &devices[j], sizeof(cl_device_id));
					strncpy(gpu[gpu_cnt].name, buffer, 99);
					found = true;
					break;
				}
			}
			free(devices);
		}

		if (!found)
			continue;

		clGetDeviceInfo(gpu[gpu_cnt].device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(size_t), &global_mem, NULL);
		applog(LOG_DEBUG, "  CL_DEVICE_GLOBAL_MEM_SIZE = %lu", global_mem);

		clGetDeviceInfo(gpu[gpu_cnt].device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &dimensions, NULL);
		applog(LOG_DEBUG, "  CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS  = %zu", dimensions);

		clGetDeviceInfo(gpu[gpu_cnt].device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_work_size, NULL);
		applog(LOG_DEBUG, "  CL_DEVICE_MAX_WORK_GROUP_SIZE  = %zu", max_work_size);

		clGetDeviceInfo(gpu[gpu_cnt].device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(size_t), &compute_units, NULL);
		applog(LOG_DEBUG, "  CL_DEVICE_MAX_COMPUTE_UNITS  = %zu", compute_units);



		//sizes = (size_t*)malloc(sizeof(size_t)*dimensions);

		//clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*dimensions, sizes, NULL);
		//for (i = 0; i < dimensions; ++i)
		//	applog(LOG_DEBUG, "  CL_DEVICE_MAX_WORK_ITEM_SIZES dim %d = %zu", i, sizes[i]);

		// Calculate Num Threads For This Device
		// GLOB MEM NEEDED: 96 + (x * VM_MEMORY_SIZE * sizeof(int32_t)) + (x * sizeof(uint32_t))
		double calc = ((double)global_mem - 96.0 - 650 * 1024 * 1024 /*Some 650 M space for who knows what*/) / ((double)VM_MEMORY_SIZE * sizeof(int32_t) + sizeof(int32_t));
		size_t bound = (size_t)calc;

		max_cores = 1024; //(compute_units - 1) * 64; // Max 64 Shaders Per Compute Unit
		gpu[gpu_cnt].threads = (bound < max_cores) ? (int)bound : max_cores;
		applog(LOG_INFO, "Global GPU Memory = %lu, Using %d Threads", global_mem, gpu[gpu_cnt].threads);

		gpu[gpu_cnt].context = clCreateContext(0, 1, &gpu[gpu_cnt].device_id, NULL, NULL, &err);
		if (!gpu[gpu_cnt].context) {
			applog(LOG_ERR, "Unable to create OpenCL context (Error: %d)", err);
			return 0;
		}

		gpu[gpu_cnt].queue = clCreateCommandQueue(gpu[gpu_cnt].context, gpu[gpu_cnt].device_id, 0, &err);
		if (!gpu[gpu_cnt].queue) {
			applog(LOG_ERR, "Unable to create OpenCL command queue (Error: %d)", err);
			return 0;
		}

		// Calculate Worksize
		size_t first_dim = (gpu[gpu_cnt].threads > max_work_size) ? max_work_size : gpu[gpu_cnt].threads;
		size_t second_dim = 1;
		if (dimensions >= 2) {
			dimensions = 2;
			second_dim = (size_t)(gpu[gpu_cnt].threads > max_work_size) ? (ceil((double)gpu[gpu_cnt].threads / (double)max_work_size)) : 1;
		}

		gpu[gpu_cnt].work_dim = dimensions;

		gpu[gpu_cnt].global_size[0] = first_dim;
		gpu[gpu_cnt].global_size[1] = second_dim;
		gpu[gpu_cnt].local_size[0] = 64;
		gpu[gpu_cnt].local_size[1] = 4;

		// Create Buffers
		gpu[gpu_cnt].vm_input = clCreateBuffer(gpu[gpu_cnt].context, CL_MEM_READ_ONLY, 96 * sizeof(char), NULL, &err);

		if (err != CL_SUCCESS) {
			applog(LOG_ERR, "ERROR: Unable to create OpenCL 'vm_input' buffer (Error: %d)", err);
			return false;
		}

		gpu[gpu_cnt].vm_mem = clCreateBuffer(gpu[gpu_cnt].context, CL_MEM_WRITE_ONLY, gpu[gpu_cnt].threads * VM_MEMORY_SIZE * sizeof(int32_t), NULL, &err);

		if (err != CL_SUCCESS) {
			applog(LOG_ERR, "ERROR: Unable to create OpenCL 'vm_mem' buffer (Error: %d)", err);
			return false;
		}

		gpu[gpu_cnt].vm_out = clCreateBuffer(gpu[gpu_cnt].context, CL_MEM_WRITE_ONLY, gpu[gpu_cnt].threads * sizeof(uint32_t), NULL, &err);

		if (err != CL_SUCCESS) {
			applog(LOG_ERR, "ERROR: Unable to create OpenCL 'vm_out' buffer (Error: %d)", err);
			return false;
		}

		gpu_cnt++;
	}

	return gpu_cnt;
}

extern bool execute_kernel(int id, const uint32_t *vm_input, uint32_t *vm_out) {
	int err;

	// Copy Random Inputs To OpenCL Buffer
	err = clEnqueueWriteBuffer(gpu[id].queue, gpu[id].vm_input, CL_TRUE, 0, 96 * sizeof(char), vm_input, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "ERROR: Unable to write to OpenCL 'vm_input' Buffer (Error: %d)", err);
		return false;
	}

	// Run OpenCL VM
	err = clEnqueueNDRangeKernel(gpu[id].queue, gpu[id].kernel_execute, gpu[id].work_dim, NULL, &gpu[id].global_size[0], &gpu[id].local_size[0], 0, NULL, NULL);
	if (err) {
		applog(LOG_ERR, "ERROR: Unable to run 'execute' kernel (Error: %d)", err);
		return false;
	}

	// Get VM Output
	err = clEnqueueReadBuffer(gpu[id].queue, gpu[id].vm_out, CL_TRUE, 0, gpu[id].threads * sizeof(uint32_t), vm_out, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		applog(LOG_ERR, "ERROR: Unable to read from OpenCL 'vm_out' Buffer (Error: %d)", err);
		return false;
	}

	return true;
}
