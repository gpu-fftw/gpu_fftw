/* vi: set expandtab sw=3: */
#include "gpu_fftw.h" /*needs to be first to define _GNU_SOURCE*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>

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

SO_LOCAL void say( const int errlvl, const char *fmt, ...)
{
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
            err_str = (char*) "gpu_fftw: INFO: ";
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
      /*if ( open("/dev/tty", O_RDWR)  < 0)
         syslog((errlvl == LOG_ERR) ? LOG_ERR : LOG_INFO, "%s", buf);
      else*/
         (void) fprintf(stderr, "%s",buf);
   }
}

bool SO_LOCAL is_gpu_active(void)
{
   char* env=NULL;
   env=getenv("GPU_FFTW_ACTIVE");
   return (env!=NULL);
}

bool SO_LOCAL gpu_active(int cmd, void* plan)
{
   char* env=NULL;
   switch(cmd) {
      case YES:
         setenv("GPU_FFTW_ACTIVE","1",1);
         break;
      case NO:
         unsetenv("GPU_FFTW_ACTIVE");
         break;
      case QUERY:
         env=getenv("GPU_FFTW_ACTIVE");
         return (env!=NULL);
      default:
         say(LOG_ERR,"Wrong command to gpu_active: %d, aborting",cmd);
         exit(1);
   }
   return true;
}
