# Run FFTW3 programs with Raspberry Pi GPU

## :gift: What you get
Use the Raspberry Pi GPU to calculate fast FFTs without changing
source code.  Binary compatibility with fftw3 programs.

:warning: This is an early alpha release, it is not production
quality. As always, use at your own risk.

## :checkered_flag: Quickstart 
Make sure you have fftw3 installed first.
```sh
git clone https://github.com/gpu-fftw/gpu_fftw.git
cd gpu_fftw
make
```
After make finishes run the following to make sure everything is working:
```sh
sudo ./gpu_fftw -D 0 -t
```
You should see something like this:
```sh
gpu_fftw - Version 0.1.1-3-gcd19

GPU FFT forward/reverse error = 0.680192ppm (nrms error)
GPU_FFTW/FFTW difference = 4.82646ppm (nrms error)
GPU FFTW 7.11652 times faster (11523.3 ffts/sec, 86.781 usec/fft, fftw3: 1619.23 ffts/sec)

Override FFT3W...PASSED
Test suite passed.
```
Notice for this run, GPU_FFTW ran 7 times faster than fftw3, it will vary
with cpu usage, fft size, etc.
Then you can run your program with:

```sh
sudo ./gpu_fftw -D 1 -d myfftprogram <args...>
```
## :mag: Why?
Thanks to the work of [Andrew Holme](http://www.aholme.co.uk/GPU_FFT/Main.htm) we
can now have fast GPU aided FFTs on the Raspberry Pi. They can be up to ten
times faster than running fftw3 by itself. However, in order
to use the GPU we have to write specialized code that makes use of the
GPU_FFT api, and many programs that are already written do not use
this api.

Most programs use the [fftw3](http://www.fftw.org) library, which is
one of the best available libraries for fft computation on a regular cpu. Even
high-end mathematical programs like octave and matlab use fftw3.

This is where the idea of GPU_FFTW  originated. The idea is to have
**binary** compatibility with fftw3. The goal is to simply install
gpu_fftw and let your programs take advantage of the GPU.

This is not easy to do, so there are some limitations, make sure
you read the [limitations] section.

## :eyes: Limitations

* **Float precission**: For now, Andrew's work only supports _float_ precision.
  Many users typically use fftw3 with double precision. Hopefully
  Andrew will add support for double precision to his work.

  To attenuate this problem, gpu_fftw supports **double squashing** which allows
  you to compute a float based fft on the GPU even if the user
  requested a double precision fft. This degraded precision may or may not
  work with your particular appplication. To enable double squashing use
  the -d option.

* **N has to be a power of two**: N for the gpu fft is limited to powers of two,
  Log2(N) has to be in the range [8,21]. gpu_fftw falls back to fftw3 if N is
  not a power of two or if it is outside the supported range.

* 2d, 3d and real FFTs are not supported yet

* **Fortran**:  A basic fortran api is provided for programs that have been
  compiled with GNU fortran. Other compilers may be added later if anyone asks.

* **Auto-fallback**: If there is any problem starting the GPU fft (e.g. lack of
  memory or permissions) gpu_fftw automatically falls back to fftw3.

* **Array copying**: gpu_fftw copies the data arrays back and forth.  It does
  it in the fastest way possible, but still needs more memory than fftw3.
  This will be fixed later on when we provide overrides for the fftw3 malloc family
  of functions.

## :bulb: TODO
- [x] ~~Fortran programs are not working (segfault), Fortran API needs to be added~~
- [ ] fftw_malloc family can be overriden to avoid copying arrays
- [ ] Test cases/benchmarks for double squashing
- [x] ~~Support double squashing~~
- [ ] r2c and c2r API missing, should we add it?
- [ ] 2d API
- [ ] Drop root permissions asap
- [ ] Fortran API for compilers other than GNU fortran
- [ ] support for fftw3_threads, so that octave works
