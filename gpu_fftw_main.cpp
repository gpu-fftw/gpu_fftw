#include <fftw3.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <complex>
#include <cassert>
#include <climits>
#include "vsn.h"

bool gpu_fftw_running(void)
{
   return getenv("GPU_FFTW_ACTIVE")!=nullptr;
}

unsigned long usec_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

unsigned long run_fft(int n,fftwf_complex* in,fftwf_complex* out,bool print,int loops=1)
{
   auto plan= fftwf_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
   auto t1 = usec_time();
   for (int i = 0; i < loops; ++i)
      fftwf_execute(plan);
   auto t2 = usec_time();

   fftwf_destroy_plan(plan);
   if (print) {
      for (int i=0; i<n; ++i) {
         std::cerr << out[i][0] << "+ i" << out[i][1] << ",\t";
      }
      std::cerr << "*************************************" << std::endl;
      std::cerr << "*************************************" << std::endl;
   }
   return t2-t1;
}

void print_test(const char* hdr,bool pass)
{
   // + Jesus is the Way, the Truth, and the Life
   std::cout << hdr << "..." << (pass ? "PASSED":" **** FAILED ****") << std::endl;
}

bool test_override_fftw3()
{
   bool pass = false;
   fftwf_complex in[256]= {1,2,3,4,5,6,7,8,9,10};
   fftwf_complex in2[256]= {1,2,3,4,5,6,7,8,9,10};
   fftwf_complex out[256];
   fftwf_complex out2[256];

   // Should run fftw plan
   run_fft(255,in,out,false);
   pass=!gpu_fftw_running();

   // Should run gpu_fftw plan
   run_fft(256,in2,out2,false);
   pass&=gpu_fftw_running();

   print_test("Override FFT3W",pass);
   return pass;
}

void show_accuracy(int N)
{
   fftwf_complex in[N]= {1,2,3,4,5,6,7,8,9,10};
   fftwf_complex out[N];
   fftwf_complex out2[N];
   const int rmax=INT_MAX/10,rmin=INT_MIN/10;

   srand((int)clock());
   for (int i = 0; i < N; ++i)
   {
      in[i][0]=rmax - rand() % rmax - rmin;
      in[i][1]=rmax - rand() % rmax - rmin;
   }

   // Should run fftw plan
   setenv("GPU_FFTW_DISABLE","1",1);
   run_fft(N,in,out,false);
   assert(gpu_fftw_running()==false);

   // Should run gpu_fftw plan
   unsetenv("GPU_FFTW_DISABLE");
   run_fft(N,in,out2,false);
   //assert(gpu_fftw_running()==true);

   double err,err_re,err_im;
   err    = 0;
   err_re = 0;
   err_im = 0;
   for (int i = 0; i < N; ++i)
   {
     err_re = out2[i][0]-out[i][0];
     err_im = out2[i][1]-out[i][1];
     err += err_re*err_re + err_im*err_im;
   }
   err = sqrt(err/N)/(rmax-rmin);

   std::string hdr=std::string("GPU_FFTW/FFTW difference = ") + std::to_string(1000000.0*err) + "ppm (nrms error)";
   std::cout << hdr << std::endl;
}

void show_speed(int N,int loops)
{
   fftwf_complex in[N]= {1,2,3,4,5,6,7,8,9,10};
   fftwf_complex out[N];
   fftwf_complex out2[N];
   const int rmax=INT_MAX/10,rmin=INT_MIN/10;

   srand((int)clock());
   for (int i = 0; i < N; ++i)
   {
      in[i][0]=rmax - rand() % rmax - rmin;
      in[i][1]=rmax - rand() % rmax - rmin;
   }

   // Should run fftw plan
   setenv("GPU_FFTW_DISABLE","1",1);
   auto fusecs = run_fft(N,in,out,false,loops);
   double fftw3_spd=loops*1000000.0/(double) fusecs;
   assert(gpu_fftw_running()==false);

   // Should run gpu_fftw plan
   unsetenv("GPU_FFTW_DISABLE");
   auto gusecs = run_fft(N,in,out2,false,loops);
   double gfftw3_spd=loops*1000000.0/(double)(gusecs);
   double gfftw3_tim=(double)(gusecs)/loops;

   //assert(gpu_fftw_running()==true);

   double err,err_re,err_im;
   err    = 0;
   err_re = 0;
   err_im = 0;
   for (int i = 0; i < N; ++i)
   {
     err_re = out2[i][0]-out[i][0];
     err_im = out2[i][1]-out[i][1];
     err += err_re*err_re + err_im*err_im;
   }
   err = sqrt(err/N)/(rmax-rmin);

   std::cout << "GPU FFTW " << (float) gfftw3_spd/fftw3_spd
      << " times faster (" << gfftw3_spd << " ffts/sec, "
      <<  gfftw3_tim << " usec/fft)"<< fftw3_spd << std::endl;
}

int main(int argc,char **argv)
{
   std::cout << "gpu_fftw - Version " << GPU_FFTW_VSN << std::endl
      << std::endl;
   test_override_fftw3();
   show_accuracy(256);
   show_speed(pow(2,10),1000);
}
