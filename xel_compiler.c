#define _GNU_SOURCE
#define __USE_GNU

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "miner.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#ifndef LM_ID_BASE
#define LM_ID_BASE              0x00
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
//	fprintf(f, "#include \"../crypto/elasticpl_crypto.h\"\n\n");
	fprintf(f, "#include \"elasticpl_crypto.h\"\n\n");
#ifdef _MSC_VER
	fprintf(f, "__declspec(thread) int32_t* mem = NULL;\n");
	fprintf(f, "__declspec(thread) uint32_t* vm_state = NULL;\n\n");
#else
	fprintf(f, "__thread int32_t* mem = NULL;\n");
	fprintf(f, "__thread uint32_t* vm_state = NULL;\n\n");
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
	fprintf(f, "\tmem = &m[0];\n");
	fprintf(f, "\tvm_state = &s[0];\n");
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

	fprintf(f, "%s", &code[0]);

	fprintf(f, "\treturn rc;\n");
	fprintf(f, "}\n");

	if (opt_test_vm) {
		fprintf(stdout, "\n********************************************************************************\n");
		fprintf(stdout, "%s", code);
		fprintf(stdout, "\n********************************************************************************\n");
	}

	fclose(f);
	free(code);
	return true;
}

bool compile_and_link(char* lib_name) {
	char str[1000];
	int ret = 0;

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
	ret = system("gcc -I./crypto -c -march=native -Ofast -msse -msse2 -msse3 -mmmx -m3dnow -DBUILDING_EXAMPLE_DLL ./work/work_lib.c -o ./work/work_lib.o");
	sprintf(str, "gcc -shared -o ./work/%s.dll ./work/work_lib.o -L./crypto -lelasticpl_crypto -lcrypto", lib_name);
	ret = system(str);
#else
#ifdef __arm__
	ret = system("gcc -I./crypto -c -std=c99 -Ofast -fPIC ./work/work_lib.c -o ./work/work_lib.o");
	sprintf(str, "gcc -std=c99 -shared -Wl,-soname,./work/%s.so.1 -o ./work/%s.so ./work/work_lib.o -L./crypto -lelasticpl_crypto -lcrypto", lib_name, lib_name);
	ret = system(str);
#else
	ret = system("gcc -I./crypto -c -march=native -Ofast -fPIC ./work/work_lib.c -o ./work/work_lib.o");
	sprintf(str, "gcc -shared -Wl,-soname,./work/%s.so.1 -o ./work/%s.so ./work/work_lib.o -L./crypto -lelasticpl_crypto -lcrypto", lib_name, lib_name);
	ret = system(str);
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
		fprintf(stderr, "Unable to load library: '%s' (Error - %d)", file_name, GetLastError());
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
	if (!inst->initialize || !inst->execute) {
		fprintf(stderr, "Unable to find library functions");
		dlclose(inst->hndl);
		exit(EXIT_FAILURE);
	}
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
		inst->initialize = 0;
		inst->execute = 0;
	}
}

/*
* The md5 kernel was heavily inspired by the md5 kernel in john the ripper
* community enhanced version. See https://github.com/magnumripper/JohnTheRipper
*
* Original software copyright (c) 2010, Dhiru Kholia
* <dhiru.kholia at gmail.com>,
* and it is hereby released to the general public under the following terms:
* Redistribution and use in source and binary forms, with or without modification,
* are permitted.
*
* Useful References:
* 1. CUDA MD5 Hashing Experiments, http://majuric.org/software/cudamd5/
* 2. oclcrack, http://sghctoma.extra.hu/index.php?p=entry&id=11
* 3. http://people.eku.edu/styere/Encrypt/JS-MD5.html
* 4. http://en.wikipedia.org/wiki/MD5#Algorithm
*/

extern bool create_opencl_source(char *work_str) {
	char *code = NULL, filename[50];
	FILE* f;

	sprintf(filename, "./work/%s.cl", work_str);

	f = fopen(filename, "w");

	if (!f)
		return false;

	fprintf(f, "#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable\n\n");

	fprintf(f, "/* The basic MD5 functions */\n");
	fprintf(f, "#define F(x, y, z)          ((z) ^ ((x) & ((y) ^ (z))))\n");
	fprintf(f, "#define G(x, y, z)          ((y) ^ ((z) & ((x) ^ (y))))\n");
	fprintf(f, "#define H(x, y, z)          ((x) ^ (y) ^ (z))\n");
	fprintf(f, "#define I(x, y, z)          ((y) ^ ((x) | ~(z)))\n\n");
	fprintf(f, "/* The MD5 transformation for all four rounds. */\n");
	fprintf(f, "#define STEP(f, a, b, c, d, x, t, s) \\\n");
	fprintf(f, "\t(a) += f((b), (c), (d)) + (x) + (t); \\\n");
	fprintf(f, "\t(a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s)))); \\\n");
	fprintf(f, "\t(a) += (b);\n\n");
	fprintf(f, "#define GET(i) (key[(i)])\n\n");
	fprintf(f, "// void md5_round(uint* internal_state, const uint* key);\n");
	fprintf(f, "static void md5_round(uint* internal_state, const uint* key) {\n");
	fprintf(f, "\tuint a, b, c, d;\n");
	fprintf(f, "\ta = internal_state[0];\n");
	fprintf(f, "\tb = internal_state[1];\n");
	fprintf(f, "\tc = internal_state[2];\n");
	fprintf(f, "\td = internal_state[3];\n\n");

	fprintf(f, "\t/* Round 1 */\n");
	fprintf(f, "\tSTEP(F, a, b, c, d, GET(0), 0xd76aa478, 7)\n");
	fprintf(f, "\tSTEP(F, d, a, b, c, GET(1), 0xe8c7b756, 12)\n");
	fprintf(f, "\tSTEP(F, c, d, a, b, GET(2), 0x242070db, 17)\n");
	fprintf(f, "\tSTEP(F, b, c, d, a, GET(3), 0xc1bdceee, 22)\n");
	fprintf(f, "\tSTEP(F, a, b, c, d, GET(4), 0xf57c0faf, 7)\n");
	fprintf(f, "\tSTEP(F, d, a, b, c, GET(5), 0x4787c62a, 12)\n");
	fprintf(f, "\tSTEP(F, c, d, a, b, GET(6), 0xa8304613, 17)\n");
	fprintf(f, "\tSTEP(F, b, c, d, a, GET(7), 0xfd469501, 22)\n");
	fprintf(f, "\tSTEP(F, a, b, c, d, GET(8), 0x698098d8, 7)\n");
	fprintf(f, "\tSTEP(F, d, a, b, c, GET(9), 0x8b44f7af, 12)\n");
	fprintf(f, "\tSTEP(F, c, d, a, b, GET(10), 0xffff5bb1, 17)\n");
	fprintf(f, "\tSTEP(F, b, c, d, a, GET(11), 0x895cd7be, 22)\n");
	fprintf(f, "\tSTEP(F, a, b, c, d, GET(12), 0x6b901122, 7)\n");
	fprintf(f, "\tSTEP(F, d, a, b, c, GET(13), 0xfd987193, 12)\n");
	fprintf(f, "\tSTEP(F, c, d, a, b, GET(14), 0xa679438e, 17)\n");
	fprintf(f, "\tSTEP(F, b, c, d, a, GET(15), 0x49b40821, 22)\n\n");

	fprintf(f, "\t/* Round 2 */\n");
	fprintf(f, "\tSTEP(G, a, b, c, d, GET(1), 0xf61e2562, 5)\n");
	fprintf(f, "\tSTEP(G, d, a, b, c, GET(6), 0xc040b340, 9)\n");
	fprintf(f, "\tSTEP(G, c, d, a, b, GET(11), 0x265e5a51, 14)\n");
	fprintf(f, "\tSTEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)\n");
	fprintf(f, "\tSTEP(G, a, b, c, d, GET(5), 0xd62f105d, 5)\n");
	fprintf(f, "\tSTEP(G, d, a, b, c, GET(10), 0x02441453, 9)\n");
	fprintf(f, "\tSTEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14)\n");
	fprintf(f, "\tSTEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)\n");
	fprintf(f, "\tSTEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5)\n");
	fprintf(f, "\tSTEP(G, d, a, b, c, GET(14), 0xc33707d6, 9)\n");
	fprintf(f, "\tSTEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14)\n");
	fprintf(f, "\tSTEP(G, b, c, d, a, GET(8), 0x455a14ed, 20)\n");
	fprintf(f, "\tSTEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5)\n");
	fprintf(f, "\tSTEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)\n");
	fprintf(f, "\tSTEP(G, c, d, a, b, GET(7), 0x676f02d9, 14)\n");
	fprintf(f, "\tSTEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)\n\n");

	fprintf(f, "\t/* Round 3 */\n");
	fprintf(f, "\tSTEP(H, a, b, c, d, GET(5), 0xfffa3942, 4)\n");
	fprintf(f, "\tSTEP(H, d, a, b, c, GET(8), 0x8771f681, 11)\n");
	fprintf(f, "\tSTEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16)\n");
	fprintf(f, "\tSTEP(H, b, c, d, a, GET(14), 0xfde5380c, 23)\n");
	fprintf(f, "\tSTEP(H, a, b, c, d, GET(1), 0xa4beea44, 4)\n");
	fprintf(f, "\tSTEP(H, d, a, b, c, GET(4), 0x4bdecfa9, 11)\n");
	fprintf(f, "\tSTEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)\n");
	fprintf(f, "\tSTEP(H, b, c, d, a, GET(10), 0xbebfbc70, 23)\n");
	fprintf(f, "\tSTEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4)\n");
	fprintf(f, "\tSTEP(H, d, a, b, c, GET(0), 0xeaa127fa, 11)\n");
	fprintf(f, "\tSTEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16)\n");
	fprintf(f, "\tSTEP(H, b, c, d, a, GET(6), 0x04881d05, 23)\n");
	fprintf(f, "\tSTEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4)\n");
	fprintf(f, "\tSTEP(H, d, a, b, c, GET(12), 0xe6db99e5, 11)\n");
	fprintf(f, "\tSTEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)\n");
	fprintf(f, "\tSTEP(H, b, c, d, a, GET(2), 0xc4ac5665, 23)\n\n");

	fprintf(f, "\t/* Round 4 */\n");
	fprintf(f, "\tSTEP(I, a, b, c, d, GET(0), 0xf4292244, 6)\n");
	fprintf(f, "\tSTEP(I, d, a, b, c, GET(7), 0x432aff97, 10)\n");
	fprintf(f, "\tSTEP(I, c, d, a, b, GET(14), 0xab9423a7, 15)\n");
	fprintf(f, "\tSTEP(I, b, c, d, a, GET(5), 0xfc93a039, 21)\n");
	fprintf(f, "\tSTEP(I, a, b, c, d, GET(12), 0x655b59c3, 6)\n");
	fprintf(f, "\tSTEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)\n");
	fprintf(f, "\tSTEP(I, c, d, a, b, GET(10), 0xffeff47d, 15)\n");
	fprintf(f, "\tSTEP(I, b, c, d, a, GET(1), 0x85845dd1, 21)\n");
	fprintf(f, "\tSTEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)\n");
	fprintf(f, "\tSTEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)\n");
	fprintf(f, "\tSTEP(I, c, d, a, b, GET(6), 0xa3014314, 15)\n");
	fprintf(f, "\tSTEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21)\n");
	fprintf(f, "\tSTEP(I, a, b, c, d, GET(4), 0xf7537e82, 6)\n");
	fprintf(f, "\tSTEP(I, d, a, b, c, GET(11), 0xbd3af235, 10)\n");
	fprintf(f, "\tSTEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)\n");
	fprintf(f, "\tSTEP(I, b, c, d, a, GET(9), 0xeb86d391, 21)\n\n");

	fprintf(f, "\tinternal_state[0] = a + internal_state[0];\n");
	fprintf(f, "\tinternal_state[1] = b + internal_state[1];\n");
	fprintf(f, "\tinternal_state[2] = c + internal_state[2];\n");
	fprintf(f, "\tinternal_state[3] = d + internal_state[3];\n");
	fprintf(f, "}\n");
	fprintf(f, "void md5(const char* restrict msg, uint length_bytes, uint* restrict out) {\n");
	fprintf(f, "\tuint i;\n");
	fprintf(f, "\tuint bytes_left;\n");
	fprintf(f, "\tchar key[64];\n\n");

	fprintf(f, "\tout[0] = 0x67452301;\n");
	fprintf(f, "\tout[1] = 0xefcdab89;\n");
	fprintf(f, "\tout[2] = 0x98badcfe;\n");
	fprintf(f, "\tout[3] = 0x10325476;\n\n");

	fprintf(f, "\tfor (bytes_left = length_bytes;  bytes_left >= 64;\n");
	fprintf(f, "\t\tbytes_left -= 64, msg = &msg[64]) {\n");
	fprintf(f, "\t\tmd5_round(out, (const uint*)msg);\n");
	fprintf(f, "\t}\n\n");

	fprintf(f, "\tfor (i = 0; i < bytes_left; i++) {\n");
	fprintf(f, "\t\tkey[i] = msg[i];\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\tkey[bytes_left++] = 0x80;\n\n");

	fprintf(f, "\tif (bytes_left <= 56) {\n");
	fprintf(f, "\t\tfor (i = bytes_left; i < 56; key[i++] = 0);\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\telse {\n");
	fprintf(f, "\t\t// If we have to pad enough to roll past this round.\n");
	fprintf(f, "\t\tfor (i = bytes_left; i < 64; key[i++] = 0);\n");
	fprintf(f, "\t\tmd5_round(out, (uint*)key);\n");
	fprintf(f, "\t\tfor (i = 0; i < 56; key[i++] = 0);\n");
	fprintf(f, "\t}\n\n");

	fprintf(f, "\tulong* len_ptr = (ulong*)&key[56];\n");
	fprintf(f, "\t*len_ptr = length_bytes * 8;\n");
	fprintf(f, "\tmd5_round(out, (uint*)key);\n");
	fprintf(f, "}\n\n");

	fprintf(f, "uint swap32(int a) {\n");
	fprintf(f, "\treturn ((a << 24) | ((a << 8) & 0x00FF0000) | ((a >> 8) & 0x0000FF00) | ((a >> 24) & 0x000000FF));\n");
	fprintf(f, "}\n\n");

	fprintf(f, "__kernel void initialize(global uint*input, global uint* base_data) {\n");
	fprintf(f, "\tint j;\n");
	fprintf(f, "\tint w = get_global_id(0); // Index in the wavefront Dim1\n");
	fprintf(f, "\tint q = get_global_id(1); // Index in the wavefront Dim2\n");
	fprintf(f, "\tint i = q*get_global_size(0)+w; // Index in the wavefront Total\n");


	fprintf(f, "\tglobal uint* pointer_to_block = &input[i * 64000];\n\n");
	fprintf(f, "\t__private uint local_hash_storage[4];\n");
	fprintf(f, "\t__private uint base_data_copy_local[20];\n\n");
	fprintf(f, "\t// 96 Bytes of base_data is made up of:\n");
	fprintf(f, "\t// 32 Byte Multiplicator (First 4 Bytes = Index, Second 4 Bytes = Incremented Value)\n");
	fprintf(f, "\t// 32 Byte Public Key\n");
	fprintf(f, "\t//  8 Byte Work ID\n");
	fprintf(f, "\t//  8 Byte Block ID\n");
	fprintf(f, "\t// 16 Byte POW Target;\n\n");
	fprintf(f, "\t// Copy base_data so the multiplicator can be incremented locally\n");
	fprintf(f, "\tfor (j = 0; j < 20; j++) // 80 bytes\n");
	fprintf(f, "\t\tbase_data_copy_local[j] = base_data[j];\n\n");
	fprintf(f, "\t// Update Index in base_data\n");
	fprintf(f, "\tbase_data_copy_local[0] = i;\n\n");
	fprintf(f, "\t// Get MD5 hash of base_data\n");
	fprintf(f, "\tmd5((char*)&base_data_copy_local[0], 80, &local_hash_storage[0]);\n\n");
	fprintf(f, "\t// Randomize The Inputs\n");
	fprintf(f, "#pragma unroll\n");
	fprintf(f, "\tfor (j = 0; j < 12; j++) {\n");
	fprintf(f, "\t\tpointer_to_block[j] = swap32(local_hash_storage[j %% 4]);\n");
	fprintf(f, "\t\tif (j > 4)\n");
	fprintf(f, "\t\t\tpointer_to_block[j] = pointer_to_block[j] ^ pointer_to_block[j - 3];\n");
	fprintf(f, "\t}\n\n");
	fprintf(f, "\t// Copy Target To Inputs;\n");
	fprintf(f, "\tfor (j = 12; j < 16; j++)\n");
	fprintf(f, "\t\tpointer_to_block[j] = base_data[j + 8];\n");
	fprintf(f, "}\n\n");

	fprintf(f, "static uint rotl32(uint x, uint n) {\n");
	fprintf(f, "\tn &= 0x0000001f;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers\n");
	fprintf(f, "\treturn (x<<n) | (x>>( (-n) & 0x0000001f ));\n");
	fprintf(f, "}\n\n");

	fprintf(f, "static uint rotr32(uint x, uint n) {\n");
	fprintf(f, "\tn &= 0x0000001f;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers\n");
	fprintf(f, "\treturn (x>>n) | (x<<( (-n) & 0x0000001f ));\n");
	fprintf(f, "}\n\n");

	fprintf(f, "#define m(x) mangle_state(x, &vm_state[0])\n\n");

	fprintf(f, "static int mangle_state(int x, uint *vm_state) {\n");
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

	fprintf(f, "__kernel void execute (global uint* input, global uint* output) {\n");
	fprintf(f, "\tint i = get_global_id(0); // this is the index in the wavefront\n");
	fprintf(f, "\tglobal uint* mem = &input[i * 64000];\n");
	fprintf(f, "\tglobal uint* out = &output[i];\n\n");
	fprintf(f, "\t__private char msg[64];\n");
	fprintf(f, "\t__private uint* msg32 = (uint*)msg;\n");
	fprintf(f, "\t__private uint hash[4];\n");
	fprintf(f, "\t__private uint target[4];\n");
	fprintf(f, "\t__private uint vm_state[4];\n\n");
	fprintf(f, "\t// Save Inputs For POW Check;\n");
	fprintf(f, "\tfor (i = 0; i < 12; i++)\n");
	fprintf(f, "\t\tmsg32[i + 4] = swap32(mem[i]);\n\n");
	fprintf(f, "\t// Get POW Target;\n");
	fprintf(f, "\tfor (i = 0; i < 4; i++)\n");
	fprintf(f, "\t\ttarget[i] = mem[i + 12];\n\n");
	fprintf(f, "\t// Reset VM State\n");
	fprintf(f, "\tvm_state[0] = 0;\n");
	fprintf(f, "\tvm_state[1] = 0;\n");
	fprintf(f, "\tvm_state[2] = 0;\n");
	fprintf(f, "\tvm_state[3] = 0;\n\n");

	fprintf(f, "\t//The following code created by ElasticPL to C parser\n");

	code = convert_ast_to_opencl();

	if (!code)
		return false;

	fprintf(f, "%s", code);

//	fprintf(f, "\tbool bounty_found=false;if (bounty_found) {\n");
//	fprintf(f, "\tif (!bounty_found) {\n");
	fprintf(f, "\tif (bounty_found) {\n");
	fprintf(f, "\t\tout[0] = 2;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\telse {\n");
	fprintf(f, "\t\tmsg32[0] = swap32(vm_state[0]);\n");
	fprintf(f, "\t\tmsg32[1] = swap32(vm_state[1]);\n");
	fprintf(f, "\t\tmsg32[2] = swap32(vm_state[2]);\n");
	fprintf(f, "\t\tmsg32[3] = swap32(vm_state[3]);\n\n");

	fprintf(f, "\t\t// Get MD5 hash of base_data\n");
	fprintf(f, "\t\tmd5((char*)&msg[0], 64, &hash[0]);\n\n");

	// Debugging
	fprintf(f, "\t\t// Temp Dump For Debugging\n");
	fprintf(f, "\t\tmem[12] = vm_state[0];\n");
	fprintf(f, "\t\tmem[13] = vm_state[1];\n");
	fprintf(f, "\t\tmem[14] = vm_state[2];\n");
	fprintf(f, "\t\tmem[15] = vm_state[3];\n\n");

	fprintf(f, "\t\tmem[16] = swap32(hash[0]);\n");
	fprintf(f, "\t\tmem[17] = swap32(hash[1]);\n");
	fprintf(f, "\t\tmem[18] = swap32(hash[2]);\n");
	fprintf(f, "\t\tmem[19] = swap32(hash[3]);\n\n");
	// Debugging

	fprintf(f, "\t\t// POW Solution Found\n");
	fprintf(f, "\t\tif (swap32(hash[0]) <= target[0])\n");
	fprintf(f, "\t\t\tout[0] = 1;\n");
	fprintf(f, "\t\telse \n");
	fprintf(f, "\t\t\tout[0] = 0;\n");
	fprintf(f, "\t}\n");
	fprintf(f, "}\n");

	if (opt_test_vm) {
		fprintf(stdout, "\n********************************************************************************\n");
		fprintf(stdout, "%s", code);
		fprintf(stdout, "\n********************************************************************************\n");
	}

	fclose(f);
	if (code != NULL)
		free(code);
	return true;
}