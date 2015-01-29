#include <fftw3.h>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <complex>
#include <cassert>
#include <climits>

#include "vsn.h"

bool quiet = false;
int dbglvl = 0;

std::string libgpufftw_file(const char* type="")
{
   return std::string(std::string("./libgpufftw") + type + ".so");
}

void vsn()
{
   if (!quiet)
      std::cout << "gpu_fftw - Version " << GPU_FFTW_VSN << std::endl
         << std::endl;
}

void usage(const char *name="gpu_fftw")
{
   vsn();
   std::cerr << "Usage: " << name << " [options] -- <program> [arguments...]"
      << std::endl;
   std::cerr << "       " << name << " -t" << std::endl;
   std::cerr <<  std::endl <<
      "        -D <level>      set debug level, 0 is default\n"
      "        -d              Run gpu fft, even if the user requested\n"
      "                        double precision. We squash doubles into\n"
      "                        floats.\n"
      "        -h              this help\n"
      "        -t              Test suite and benchmarks\n"
      "\n"
      "The first form runs <program> with gpu_fftw enabled,\n"
      "the second runs tests and prints benchmark information.\n"
      "To see if the GPU is running and when it fallbacks to fftw3 do this:\n"
      "   ./gpu_fftw -D 1 -d -- <your_fftw3_program> [your_program_arguments]\n";
}

// Argc and argv are the values for the program to be executed
void exec_other(int argc, char* argv[],char *envp[],bool dblsquash)
{
   const int MAXSIZE = 512;
   char* newenvp[MAXSIZE];
   std::string tmp("LD_PRELOAD=");
   tmp+=libgpufftw_file("f");
   if (dblsquash) {
      tmp+=":" + libgpufftw_file();
   }

   //Copy environment and add LD_PRELOAD
   int i=0;
   while (envp[i] && i < MAXSIZE - 3) {
      newenvp[i] = envp[i];
      i++;
   }
   newenvp[i++] = const_cast<char*>(tmp.c_str());
   if (dbglvl>0) {
      tmp = "GPU_FFTW_DEBUG=" + std::to_string(dbglvl+5);
      newenvp[i++] = const_cast<char*>(tmp.c_str());
   }
   newenvp[i] = nullptr;

   if (argc < 1) {
      usage();
      return;
   }

   if (!quiet) {
      std::cerr << "Running '" << argv[0] << " ";
      i=1;
      while (argv[i]) {
         std::cerr << argv[i] << ((i==argc-1) ? "":" ");
         i++;
      }
      std::cerr << "'"
         << (dblsquash ? " with double squash enabled":"") << std::endl;
   }

   execve(argv[0], &(argv[0]), newenvp);

   // execve() only returns on error
   std::cerr << "Error executing " << argv[0] << ": '"
      << strerror(errno) << "'" << std::endl;
}

/************************
 *  Tests & Benchmarks  *
 ************************/
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

unsigned long run_fft(int n,fftwf_complex* in,fftwf_complex* out,int direction,bool print,int loops=1)
{
   auto plan= fftwf_plan_dft_1d(n, in, out, direction, FFTW_ESTIMATE);
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
   run_fft(255,in,out,FFTW_FORWARD,false);
   pass=!gpu_fftw_running();

   // Should run gpu_fftw plan
   run_fft(256,in2,out2,FFTW_FORWARD,false);
   pass&=gpu_fftw_running();

   print_test("Override FFT3W",pass);
   return pass;
}

void show_fwd_rev(int N)
{
   fftwf_complex in[N]={1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0};
   fftwf_complex out[N];
   fftwf_complex out2[N];
   const int rmax=INT_MAX/10,rmin=INT_MIN/10;

   srand((int)clock());
   for (int i = 0; i < N; ++i)
   {
      in[i][0]=rmax - rand() % rmax - rmin;
      in[i][1]=rmax - rand() % rmax - rmin;
   }

   double invN=1.0/(double)N;
   run_fft(N,in,out,FFTW_FORWARD,false);
   for (int i = 0; i < N; ++i)
   {
     out[i][0]*=invN;
     out[i][1]*=invN;
   }
   run_fft(N,out,out2,FFTW_BACKWARD,false);

   double err,err_re,err_im;
   err    = 0;
   err_re = 0;
   err_im = 0;
   for (int i = 0; i < N; ++i)
   {
      err_re = out2[i][0] - in[i][0];
      err_im = out2[i][1] - in[i][1];
      err += err_re*err_re + err_im*err_im;
   }
   err = sqrt(err/N)/(rmax-rmin);

   std::cout << "GPU FFT forward/reverse error = " << 1000000.0*err << "ppm (nrms error)"
      << std::endl;
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
   run_fft(N,in,out,FFTW_FORWARD,false);
   assert(gpu_fftw_running()==false);

   // Should run gpu_fftw plan
   unsetenv("GPU_FFTW_DISABLE");
   run_fft(N,in,out2,FFTW_FORWARD,false);
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

   std::cout << "GPU_FFTW/FFTW difference = " << 1000000.0*err << "ppm (nrms error)"
      << std::endl;
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
   auto fusecs = run_fft(N,in,out,FFTW_FORWARD,false,loops);
   double fftw3_spd=loops*1000000.0/(double) fusecs;
   assert(gpu_fftw_running()==false);

   // Should run gpu_fftw plan
   unsetenv("GPU_FFTW_DISABLE");
   auto gusecs = run_fft(N,in,out2,FFTW_FORWARD,false,loops);
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
      <<  gfftw3_tim << " usec/fft, fftw3: "<< fftw3_spd
      << " ffts/sec)" << std::endl;
}


bool tests()
{
   bool pass=true;
   show_fwd_rev(256);
   show_accuracy(256);
   show_speed(pow(2,10),1000);
   std::cerr << std::endl;
   pass&=test_override_fftw3();

   if (pass)
      std::cerr << "Test suite passed." << std::endl;
   else
      std::cerr << "Test suite ***** FAILED" << std::endl;
   return pass;
}

int main(int argc,char **argv, char* envp[])
{
   int opt;
   bool dblsquash=false;
   char *test_argv[] = { argv[0], (char*) "-z", nullptr };

   if (argc < 2) {
      usage(argv[0]);
      return 2;
   }

   while ((opt = getopt(argc, argv, "ztqhdD:")) != -1) {
      switch (opt) {
         case 'z': //Used only internally
            return tests() ? 0:1;
         case 't':
            vsn();
            quiet=true;
            exec_other(2,test_argv,envp,true);
            return 3;
            break;
         case 'q':
            quiet = true;
            break;
         case 'D':
            dbglvl= std::stoi(optarg);
            break;
         case 'd':
            dblsquash=true;
            break;
         case 'h':
            usage(argv[0]);
            return 2;
      }
   }

   vsn();
   exec_other(argc-optind,&argv[optind],envp,dblsquash);
   return 3; //if exec_other returns it is an error
}
