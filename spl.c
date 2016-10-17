#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int execute();

int32_t mem[64000];
uint64_t vm_state1 = 0;
uint64_t vm_state2 = 0;

int run_spl(int32_t *input) {
        int i, rc;
        for (i=0; i<12; i++)
                mem[i] = input[i];
        rc = execute();
//      printf("MANGLE STATE: %lld %lld.\n",vm_state1,vm_state2);
        printf("Inputs: %d %d\n",input[0],input[1]);
        return rc;
}

uint64_t rotl64 (uint64_t x, unsigned int n)
{
        const unsigned int mask = (CHAR_BIT*sizeof(x)-1);
        n &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
        return (x<<n) | (x>>( (-n)&mask ));
}

uint64_t rotr64 (uint64_t x, unsigned int n)
{
        const unsigned int mask = (CHAR_BIT*sizeof(x)-1);
        n &= mask;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
        return (x>>n) | (x<<( (-n)&mask ));
}

int m(int x) {
//      printf("MANGLE %d\n",x);
        int mod = x % 64;
        if (x % 2 == 0) {
                vm_state1 = rotl64(vm_state1, mod);
                vm_state1 = vm_state1 ^ x;
        }
        else {
                vm_state2 = rotr64(vm_state2, mod);
                vm_state2 = vm_state2 ^ x;
        }
        return x;
}

int execute() {
        vm_state1=0;
        vm_state2=0;
        return 0;
}
 