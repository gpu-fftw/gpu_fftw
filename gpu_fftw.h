#define _GNU_SOURCE /*for RTLD_NEXT see dlopen(3) */
#include <dlfcn.h>
#include <stdbool.h>
#include "hello_fft/gpu_fft.h"

#define MPLAN_ARRSIZE 1024
#define SO_EXPORT __attribute__ ((visibility ("default")))
#define SO_LOCAL  __attribute__ ((visibility ("hidden")))
#define YES 0
#define NO 1
#define QUERY 2

/* see http://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2 */
#define IS_2_PWR(x) (((x) & (x-1)) == 0)

struct GPU_FFTW_PLAN { // Opaque to the user, don't use internals
   void* plan;
   void* out;
   int n;
   bool gpu;
   int* count_ptr;
};

extern SO_LOCAL void say( const int errlvl, const char *fmt, ...);
extern SO_LOCAL void* orig_func(const char* oname,const void* curr_fun);
extern SO_LOCAL unsigned int log2u(unsigned int i);
extern SO_LOCAL void gpu_active(bool yesno);
