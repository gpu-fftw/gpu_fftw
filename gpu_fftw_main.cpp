#include <fftw3.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <complex>

bool gpu_fftw_running(void)
{
   return std::getenv("GPU_FFTW_ACTIVE")!=nullptr;
}

void run_fft(int n,fftwf_complex* in,fftwf_complex* out,bool print)
{
   auto plan= fftwf_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
   fftwf_execute(plan);
   fftwf_destroy_plan(plan);
   if (print) {
      for (int i=0; i<n; ++i) {
         std::cerr << out[i][0] << "+ i" << out[i][1] << ",\t";
      }
      std::cerr << "*************************************" << std::endl;
      std::cerr << "*************************************" << std::endl;
   }
}

int main(int argc,char **argv)
{
   bool pass;

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

   // Print test result
   if (pass) {
      std::cerr << "Test passed" << std::endl;
      return 0;
   } else {
      std::cerr << "Test FAILED" << std::endl;
      return 1;
   }
}
