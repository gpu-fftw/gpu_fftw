/* vi: set expandtab sw=3: */
#include "gpu_fftw.h" /*needs to be first to define _GNU_SOURCE*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifndef S_SPLINT_S
#include <syslog.h>
#endif



/* see http://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2 */
#define IS_2_PWR(x) (((x) & (x-1)) == 0)

/* a fingerprint is used to distinguish gpu_fftw pointers from fftw3 pointers
 * in order to prevent segfaults */
SO_LOCAL bool fingerprint_ok(void *p)
{
   static unsigned char fprint[FINGERPRINTSZ] = FINGERPRINT;
   return !memcmp(p,(void*)fprint,FINGERPRINTSZ);
}

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

   say(LOG_DEBUG,"original %s(...) -> %p\n",oname,ofun_ptr);
   say(LOG_DEBUG,"gpu      %s(...) -> %p\n",oname,curr_fun);

   error = dlerror();
   if (error != NULL) {
      say(LOG_ERR,"%s\n", error);
      return NULL;
   }

   assert(curr_fun != ofun_ptr);
   assert(ofun_ptr != NULL);
   return ofun_ptr;
}

SO_LOCAL void say( const int errlvl, const char *fmt, ...)
{
   static int is_ctty = -1;

   /* returns 0 if there is no number in then env. variable*/
   char *envdbg = getenv("GPU_FFTW_DEBUG");
	char buf[1024];
	char *err_str;
	va_list ap;
   int dbglvl = 0;

   if (envdbg != NULL)
      dbglvl = atoi(envdbg);

   if (dbglvl >= errlvl) {
      switch ( errlvl ) {
         case LOG_ERR:
            err_str = (char*) "gpu_fftw: ERROR: ";
            break;
         case LOG_INFO:
            err_str = (char*) "gpu_fftw:  INFO: ";
            break;
         case LOG_DEBUG:
            err_str = (char*) "gpu_fftw: DEBUG: ";
            break;
         default:
            err_str = (char*) "gpu_fftw: ";
      }

      buf[0] = '\0';
      (void)strncat(buf, err_str, sizeof(buf)-1);

      va_start(ap, fmt);
      vsnprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), fmt, ap);
      va_end(ap);

      if (is_ctty==-1) {
         int res=open("/proc/self/fd/0", O_NOCTTY|O_NOFOLLOW);
         if (res>0) // should never happen b/c proc/self/fd/0 is a symlink
            is_ctty = 1;
         else
            if (errno==ELOOP) // we have a symlink
               is_ctty = 1;
            else
               is_ctty = 0;
      }

      if (!is_ctty)
         syslog((errlvl == LOG_ERR) ? LOG_ERR : LOG_INFO, "%s", buf);
      else
         (void) fprintf(stderr, "%s",buf);
   }
}

void SO_LOCAL gpu_active(bool yesno)
{
   if (yesno)
      setenv("GPU_FFTW_ACTIVE","1",1);
   else
      unsetenv("GPU_FFTW_ACTIVE");
}

