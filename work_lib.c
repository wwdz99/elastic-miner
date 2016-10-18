#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <malloc.h>
__declspec(dllexport) int32_t* mem = 0;
__declspec(dllexport) uint32_t vm_state1 = 0;
__declspec(dllexport) uint32_t vm_state2 = 0;
__declspec(dllexport) uint32_t vm_state3 = 0;
__declspec(dllexport) uint32_t vm_state4 = 0;
#ifdef _WIN32
#define ALLOC_ALIGNED_BUFFER(_numBytes) ((int *)_aligned_malloc (_numBytes, 64))
#define FREE_ALIGNED_BUFFER(_buffer) _aligned_free(_buffer)
#elif __SSE__
// allocate memory aligned to 64-bytes memory boundary
#define ALLOC_ALIGNED_BUFFER(_numBytes) (int *) _mm_malloc(_numBytes, 64)
#define FREE_ALIGNED_BUFFER(_buffer) _mm_free(_buffer)
#else
// NOTE(mhroth): valloc seems to work well, but is deprecated!
#define ALLOC_ALIGNED_BUFFER(_numBytes) (int *) valloc(_numBytes)
#define FREE_ALIGNED_BUFFER(_buffer) free(_buffer)
#endif
static const unsigned int mask32 = (CHAR_BIT*sizeof(uint32_t)-1);
static inline uint32_t rotl32 (uint32_t x, unsigned int n)
{
  n &= mask32;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
  return (x<<n) | (x>>( (-n)&mask32 ));
}
static inline uint32_t rotr32 (uint32_t x, unsigned int n)
{
  n &= mask32;  // avoid undef behaviour with NDEBUG.  0 overhead for most types / compilers
  return (x>>n) | (x<<( (-n)&mask32 ));
}
static int m(int x) {
   int mod = x % 32;
   int leaf = mod % 4;
   if (leaf == 0) {
       vm_state1 = rotl32(vm_state1, mod);
        vm_state1 = vm_state1 ^ x;
   }
   else if (leaf == 1) {
       vm_state2 = rotl32(vm_state2, mod);
       vm_state2 = vm_state2 ^ x;
   }
   else if (leaf == 2) {
       vm_state3 = rotl32(vm_state3, mod);
       vm_state3 = vm_state3 ^ x;
   }
   else {
       vm_state4 = rotr32(vm_state4, mod);
       vm_state4 = vm_state4 ^ x;
   }
    return x;
}
__declspec(dllexport) int fill_ints(int input[]){
  if(mem==0)
  mem=ALLOC_ALIGNED_BUFFER(64000*sizeof(int));
   for(int i=0;i<12;++i)
    mem[i] = input[i];
vm_state1=0;
vm_state2=0;
vm_state3=0;
vm_state4=0;
return 0;
}
__declspec(dllexport) int execute(){
vm_state1=0;
vm_state2=0;
vm_state3=0;
vm_state4=0;
mem[m(1)] = m(1);
mem[m(2)] = m(1);
mem[m(3)] = m(1);
mem[m(4)] = m(1);
mem[m(5)] = m(1);
if ((m(((1) == (2))?1:0)) !=0 ) {mem[m(6)] = m(1);
mem[m(7)] = m(2);
}
return m((m(((1) == (1))?1:0))!=0?1:0);}

void free_vm(){
  if(mem!=0)
    FREE_ALIGNED_BUFFER(mem);
}

