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
	FILE* f = fopen("./work/work_lib.c", "w");

	if (!f)
		return false;

	fprintf(f, "#include <stdbool.h>\n");
	fprintf(f, "#include <stdio.h>\n");
	fprintf(f, "#include <stdint.h>\n");
	fprintf(f, "#include <stdlib.h>\n");
	fprintf(f, "#include <limits.h>\n");
	fprintf(f, "#include <time.h>\n");
	fprintf(f, "#include \"../crypto/elasticpl_crypto.h\"\n\n");
	fprintf(f, "int32_t* mem = NULL;\n");
	fprintf(f, "uint32_t* vm_state = NULL;\n\n");
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
	fprintf(f, "\t\tvm_state[0] = rotl32(vm_state[0], mod);\n");
	fprintf(f, "\t\tvm_state[0] = vm_state[0] ^ x;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\telse if (leaf == 1) {\n");
	fprintf(f, "\t\tvm_state[1] = rotl32(vm_state[1], mod);\n");
	fprintf(f, "\t\tvm_state[1] = vm_state[1] ^ x;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\telse if (leaf == 2) {\n");
	fprintf(f, "\t\tvm_state[2] = rotl32(vm_state[2], mod);\n");
	fprintf(f, "\t\tvm_state[2] = vm_state[2] ^ x;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\telse {\n");
	fprintf(f, "\t\tvm_state[3] = rotl32(vm_state[3], mod);\n");
	fprintf(f, "\t\tvm_state[3] = vm_state[3] ^ x;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\treturn x;\n");
	fprintf(f, "}\n\n");
#ifdef WIN32
	fprintf(f, "__declspec(dllexport) void initialize(int32_t* m, uint32_t* s) {\n");
#else
	fprintf(f, "void initialize(int32_t* m, uint32_t* s) {\n");
#endif
	fprintf(f, "\tmem = m;\n");
	fprintf(f, "\tvm_state = s;\n");
	fprintf(f, "}\n\n");
#ifdef WIN32
	fprintf(f, "__declspec(dllexport) int execute() {\n");
#else
	fprintf(f, "int execute() {\n");
#endif
	fprintf(f, "\tint rc;\n\n");
	fprintf(f, "//The following code created by ElasticPL to C parser\n");

	code = convert_ast_to_c();

	if (!code)
		return false;

	fprintf(f, &code[0]);

	fprintf(f, "\treturn rc;\n");
	fprintf(f, "}\n");

	if (opt_test_vm) {
		fprintf(stdout, "\n********************************************************************************\n");
		fprintf(stdout, code);
		fprintf(stdout, "\n********************************************************************************\n");
	}

	fclose(f);
	free(code);
	return true;
}

bool compile_and_link(char* lib_name) {
	char str[1000];

	applog(LOG_DEBUG, "DEBUG: Converting ElasticPL to C");

	if (!create_c_source()) {
		applog(LOG_ERR, "Unable to convert ElasticPL to C");
		return false;
	}

	applog(LOG_DEBUG, "DEBUG: Compiling C Library: %s", lib_name);

#ifdef _MSC_VER
	sprintf(str, "compile_dll.bat ./work/%s.dll", lib_name);
	system(str);
#else
 #ifdef __MINGW32__
	system("gcc -c -march=native -Ofast -msse -msse2 -msse3 -mmmx -m3dnow -DBUILDING_EXAMPLE_DLL ./work/work_lib.c -o ./work/work_lib.o");
	sprintf(str, "gcc -shared -o ./work/%s.dll ./work/work_lib.o -L./crypto -lelasticpl_crypto -lcrypto", lib_name);
	system(str);
 #else
  #ifdef __arm__
	system("gcc -c -std=c99 -Ofast -fPIC ./work/work_lib.c -o ./work/work_lib.o");
	sprintf(str, "gcc -std=c99 -shared -Wl,-soname,./work/%s.so.1 -o ./work/%s.so ./work/work_lib.o -L./crypto -lelasticpl_crypto -lcrypto", lib_name, lib_name);
	system(str);
  #else
	system("gcc -c -march=native -Ofast -fPIC ./work/work_lib.c -o ./work/work_lib.o");
	sprintf(str, "gcc -shared -Wl,-soname,./work/%s.so.1 -o ./work/%s.so ./work/work_lib.o -L./crypto -lelasticpl_crypto -lcrypto", lib_name, lib_name);
	system(str);
  #endif
 #endif
#endif

	return true;
}

void create_instance(struct instance* inst, char *lib_name) {
	char file_name[1000];
#ifdef WIN32
	sprintf(file_name, "./work/%s.dll", lib_name);
	inst->hndl = LoadLibrary(file_name);
	if (!inst->hndl) {
		fprintf(stderr, "Unable to load library: '%s'", file_name);
		exit(EXIT_FAILURE);
	}
	inst->initialize = (int(__cdecl *)(int32_t *, uint32_t *))GetProcAddress((HMODULE)inst->hndl, "initialize");
	inst->execute = (int(__cdecl *)())GetProcAddress((HMODULE)inst->hndl, "execute");
	if (!inst->initialize || !inst->execute) {
		fprintf(stderr, "Unable to find library functions");
		FreeLibrary((HMODULE)inst->hndl);
		exit(EXIT_FAILURE);
	}
	applog(LOG_DEBUG, "DEBUG: '%s' Loaded", file_name);
#else
	sprintf(file_name, "./work/%s.so", lib_name);
	inst->hndl = dlmopen(LM_ID_BASE, file_name, RTLD_NOW);
	if (!inst->hndl) {
		fprintf(stderr, "%sn", dlerror());
		exit(EXIT_FAILURE);
	}
	inst->initialize = dlsym(inst->hndl, "initialize");
	inst->execute = dlsym(inst->hndl, "execute");
#endif

	inst->initialize(vm_mem, vm_state);
}

void free_compiler(struct instance* inst) {
	if (inst->hndl != 0) {
#ifdef WIN32
		FreeLibrary((HMODULE)inst->hndl);
#else
		dlclose(inst->hndl);
#endif
		inst->hndl = 0;
		inst->initialize = 0;
		inst->execute = 0;
	}
}