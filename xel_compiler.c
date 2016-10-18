
#include "miner.h"
#define _GNU_SOURCE
#define __USE_GNU
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef WIN32
#include <dlfcn.h>
#endif

bool compile_and_link(char* source_code) {
	FILE* tempfile = fopen("work_lib.spl", "w+");
	fprintf(tempfile, "%s", source_code);
	fclose(tempfile);

#ifdef WIN32
//
//
// Need to add C parser here when complete.
//
	system("compile_dll.bat");
//
//
//
//
#else
	FILE* tempfile2 = fopen("work_lib.c", "w+");
	char partialcode[600000];
	FILE *f = popen("java -jar ElasticToCCompiler.jar work_lib.spl", "r");
	if (f == 0) {
		printf("Cannot open spl file");
		exit(1);
	}
	while (fgets(partialcode, 600000, f) != NULL) {
		fprintf(tempfile2, "%s", partialcode);
	}
	fclose(tempfile2);

	pclose(f);

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