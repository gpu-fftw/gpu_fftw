#include <fftw3.h>
#include <cstdlib>
#include <string>
#include <iostream>

bool gpu_fftw_running(void)
{
   return std::getenv("GPU_FFTW_ACTIVE")!=nullptr;
}

int main(int argc,char **argv)
{
   bool pass;

   // Should run fftw plan
   auto plan= fftwf_plan_dft_1d(1, NULL, NULL, FFTW_FORWARD, FFTW_ESTIMATE);
   fftwf_destroy_plan(plan);
   pass=!gpu_fftw_running();

   // Should run gpu_fftw plan
   (void) fftwf_plan_dft_1d(256, NULL, NULL, FFTW_FORWARD, FFTW_ESTIMATE);
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
