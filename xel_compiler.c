#define _GNU_SOURCE
#define __USE_GNU

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "miner.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

bool create_c_source() {
	char *code;
	FILE* f = fopen("work_lib.c", "w");

	if (!f)
		return false;

	fprintf(f, "#include <stdio.h>\n");
	fprintf(f, "#include <stdint.h>\n");
	fprintf(f, "#include <stdlib.h>\n");
	fprintf(f, "#include <limits.h>\n");
	fprintf(f, "#include <time.h>\n");
#ifdef WIN32
	fprintf(f, "#include <malloc.h>\n\n");
#else
	fprintf(f, "#include <mm_malloc.h>\n\n");
#endif
	fprintf(f, "int32_t* mem = 0;\n");
#ifdef WIN32
	fprintf(f, "__declspec(dllexport) uint32_t vm_state1 = 0;\n");
	fprintf(f, "__declspec(dllexport) uint32_t vm_state2 = 0;\n");
	fprintf(f, "__declspec(dllexport) uint32_t vm_state3 = 0;\n");
	fprintf(f, "__declspec(dllexport) uint32_t vm_state4 = 0;\n\n");
	fprintf(f, "#define ALLOC_ALIGNED_BUFFER(_numBytes) ((int *)_aligned_malloc (_numBytes, 64))\n");
	fprintf(f, "#define FREE_ALIGNED_BUFFER(_buffer) _aligned_free(_buffer)\n\n");
#else
	fprintf(f, "uint32_t vm_state1 = 0;\n");
	fprintf(f, "uint32_t vm_state2 = 0;\n");
	fprintf(f, "uint32_t vm_state3 = 0;\n");
	fprintf(f, "uint32_t vm_state4 = 0;\n\n");
	fprintf(f, "#define ALLOC_ALIGNED_BUFFER(_numBytes) (int *) _mm_malloc(_numBytes, 64)\n");
	fprintf(f, "#define FREE_ALIGNED_BUFFER(_buffer) _mm_free(_buffer)\n");
#endif
	fprintf(f, "static const unsigned int mask32 = (CHAR_BIT*sizeof(uint32_t)-1);\n\n");
	fprintf(f, "static uint32_t rotl32 (uint32_t x, unsigned int n) {\n");
	fprintf(f, "\tn &= mask32;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers\n");
	fprintf(f, "\treturn (x<<n) | (x>>( (-n)&mask32 ));\n");
	fprintf(f, "}\n\n");
	fprintf(f, "static uint32_t rotr32 (uint32_t x, unsigned int n) {\n");
	fprintf(f, "\tn &= mask32;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers\n");
	fprintf(f, "\treturn (x>>n) | (x<<( (-n)&mask32 ));\n");
	fprintf(f, "}\n\n");
	fprintf(f, "static int m(int x) {\n");
	fprintf(f, "\tint mod = x %% 32;\n");
	fprintf(f, "\tint leaf = mod %% 4;\n");
	fprintf(f, "\tif (leaf == 0) {\n");
	fprintf(f, "\t\tvm_state1 = rotl32(vm_state1, mod);\n");
	fprintf(f, "\t\tvm_state1 = vm_state1 ^ x;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\telse if (leaf == 1) {\n");
	fprintf(f, "\t\tvm_state2 = rotl32(vm_state2, mod);\n");
	fprintf(f, "\t\tvm_state2 = vm_state2 ^ x;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\telse if (leaf == 2) {\n");
	fprintf(f, "\t\tvm_state3 = rotl32(vm_state3, mod);\n");
	fprintf(f, "\t\tvm_state3 = vm_state3 ^ x;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\telse {\n");
	fprintf(f, "\t\tvm_state4 = rotl32(vm_state4, mod);\n");
	fprintf(f, "\t\tvm_state4 = vm_state4 ^ x;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\treturn x;\n");
	fprintf(f, "}\n\n");
#ifdef WIN32
	fprintf(f, "__declspec(dllexport) void fill_ints(int input[]) {\n");
#else
	fprintf(f, "void fill_ints(int input[]) {\n");
#endif
	fprintf(f, "\nint i;\n");
	fprintf(f, "\tif (mem == 0)\n");
	fprintf(f, "\t\tmem = ALLOC_ALIGNED_BUFFER(64000 * sizeof(int));\n");
	fprintf(f, "\tfor (i = 0; i < 12; i++)\n");
	fprintf(f, "\t\tmem[i] = input[i];\n");
	fprintf(f, "\tvm_state1=0;\n");
	fprintf(f, "\tvm_state2=0;\n");
	fprintf(f, "\tvm_state3=0;\n");
	fprintf(f,"\tvm_state4=0;\n" );
	fprintf(f, "}\n\n");
#ifdef WIN32
	fprintf(f, "__declspec(dllexport) int execute() {\n");
#else
	fprintf(f, "int execute() {\n");
#endif
	fprintf(f, "\tvm_state1=0;\n");
	fprintf(f, "\tvm_state2=0;\n");
	fprintf(f, "\tvm_state3=0;\n");
	fprintf(f, "\tvm_state4=0;\n\n");
	fprintf(f, "//The following code created by ElasticPL to C parser\n");

	code = convert_ast_to_c();

	if (!code)
		return false;

	fprintf(f, &code[0]);
	fprintf(f, "}\n");

	if (opt_test_compiler) {
		fprintf(stdout, "\n********************************************************************************\n");
		fprintf(stdout, code);
		fprintf(stdout, "\n********************************************************************************\n");
	}

	fclose(f);
	free(code);
	return true;
}

bool compile_and_link(char* source_code) {

	if (!create_c_source()) {
			fprintf(stderr, "Unable to convert ElasticPL to C - \n%s\n", source_code);
			exit(EXIT_FAILURE);
	}

#ifdef WIN32
	system("compile_dll.bat");
#else
	system("gcc -c -march=native -Ofast -fPIC work_lib.c -o work_lib.o");
	system("gcc -shared -Wl,-soname,work_lib.so.1 -o work_lib.so work_lib.o");
#endif

	return true;
}

void create_instance(struct instance* inst) {
#ifdef WIN32
	inst->hndl = LoadLibrary("work_lib.dll");
	if (!inst->hndl) {
		fprintf(stderr, "Unable to load library: 'work_lib.dll'");
		exit(EXIT_FAILURE);
	}
	inst->fill_ints = (int(__cdecl *)(int *))GetProcAddress((HMODULE)inst->hndl, "fill_ints");
	inst->execute = (int(__cdecl *)())GetProcAddress((HMODULE)inst->hndl, "execute");
	inst->vm_state1 = (uint32_t *)GetProcAddress((HMODULE)inst->hndl, "vm_state1");
	inst->vm_state2 = (uint32_t *)GetProcAddress((HMODULE)inst->hndl, "vm_state2");
	inst->vm_state3 = (uint32_t *)GetProcAddress((HMODULE)inst->hndl, "vm_state3");
	inst->vm_state4 = (uint32_t *)GetProcAddress((HMODULE)inst->hndl, "vm_state4");
	if (!inst->fill_ints || !inst->execute || !inst->vm_state1 || !inst->vm_state2 || !inst->vm_state3 || !inst->vm_state4) {
		fprintf(stderr, "Unable to find library functions / variables");
		FreeLibrary((HMODULE)inst->hndl);
		exit(EXIT_FAILURE);
	}
	applog(LOG_DEBUG, "DEBUG: 'work_lib.dll' Loaded");
#else
	inst->hndl = dlmopen(LM_ID_BASE, "./work_lib.so", RTLD_NOW);
	if (!inst->hndl) {
		fprintf(stderr, "%sn", dlerror());
		exit(EXIT_FAILURE);
	}
	inst->fill_ints = dlsym(inst->hndl, "fill_ints");
	inst->execute = dlsym(inst->hndl, "execute");
	inst->vm_state1 = dlsym(inst->hndl, "vm_state1");
	inst->vm_state2 = dlsym(inst->hndl, "vm_state2");
	inst->vm_state3 = dlsym(inst->hndl, "vm_state3");
	inst->vm_state4 = dlsym(inst->hndl, "vm_state4");
#endif
}

void free_compiler(struct instance* inst) {
	if (inst->hndl != 0) {
#ifdef WIN32
		FreeLibrary((HMODULE)inst->hndl); 
#else
		dlclose(inst->hndl);
#endif
		inst->hndl = 0;
		inst->fill_ints = 0;
		inst->execute = 0;
		inst->vm_state1 = 0;
		inst->vm_state2 = 0;
		inst->vm_state3 = 0;
		inst->vm_state4 = 0;
	}
}