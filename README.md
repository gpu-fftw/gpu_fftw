# Raspberry Pi gpu fftw3 drop-in replacement

## What you get
Use the Raspberry Pi GPU to calculate fast FFTs without changing
source code.  Binary compatibility with fftw3.

:warning: This is an early alpha release, it is not productiong
quality. As always, use at your own risk.

## How to use it
./gpu_fftw -d myfftprogram <args...>

## Why?
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

## :stop: Limitations

* *Float precission*: For now, Andrew's work only supports _float_ precision.
  Many users typically use fftw3 with double precision. Hopefully
  Andrew will add support for double precision to his work.

  To attenuate this problem, gpu_fftw supports **double squashing** which allows
  you to compute a float based fft on the GPU even if the user
  requested a double precision fft. This degraded precision may or may not
  work with your particular appplication. To enable double squashing use
  the -d option.

* **N has to be a power of two**: N for the fft is limited to powers of two,
  Log2(N) has to be in the range [8,21]. gpu_fftw falls back to fftw3 if N is
  not a power of two or if it is
  outside the supported range.

* 2d, 3d and real FFTs are not supported yet

* If there is any problem starting the GPU fft (e.g. lack of memory or
  permissions) gpu_fftw automatically falls back to fftw3.

## TODO
- [ ] Fortran programs are not working (segfault), Fortran API needs to be added
- [ ] fftw_malloc family can be overriden to avoid copying arrays
- [ ] Test cases/benchmarks for float squashing
- [x] ~~Support double squashing~~
- [ ] r2c and c2r API missing, should we add it?
- [ ] 2d API



