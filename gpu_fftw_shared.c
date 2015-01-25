/* vi: set ft=c: */
#include "gpu_fftw.h"
#include "hello_fft/gpu_fft.h"
#include "hello_fft/mailbox.h"
#include <stdlib.h>
#include <fftw3.h>
#include <assert.h>

#ifndef S_SPLINT_S
#include <syslog.h>
#endif

/* see http://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2 */
#define IS_2_PWR(x) (((x) & (x-1)) == 0)

/* This function relies on the fact that i>=2 and is a power of 2 */
SO_LOCAL unsigned int log2u(unsigned int i)
{
   /* The integer log2 of an unsigned int
    * is the highest bit position set to 1) */
   unsigned int bit = 0;
   assert(i>=2);
   while (i >>= 1)  ++bit;
   return bit;
}

SO_LOCAL void* orig_func(const char* oname,const void* curr_fun)
{
   char *error;
   void* ofun_ptr;

   (void) dlerror();
   ofun_ptr = dlsym(RTLD_NEXT,oname);

   say(LOG_DEBUG,"original %s -> %p\n",oname,ofun_ptr);
   say(LOG_DEBUG,"gpu %s -> %p\n",oname,curr_fun);

   error = dlerror();
   if (error != NULL) {
      say(LOG_ERR,"%s\n", error);
      return NULL;
   }

   assert(curr_fun != ofun_ptr);
   assert(ofun_ptr != NULL);
   return ofun_ptr;
}

