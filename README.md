# Raspberry Pi gpu fftw3 drop-in replacement

## What you get
Use the Raspberry Pi GPU to calculate fast FFTs without touching
source code.  Binary compatibility with fftw3.

:warning: This is an early alpha release, it is not productiong
quality. As always, use at your own risk.

## How to use it
TBD

## Why?
Thanks to the work of [Andrew Holme](http://www.aholme.co.uk/GPU_FFT/Main.htm) we
can now have fast GPU aided FFTs on the Raspberry Pi. They can be up to ten
times faster than running fftw3 by iself. However, in order
to use the GPU we have to write specialized code that makes use of the
GPU_FFT api, and many programs that are already written do not use
this API.g

Most programs use the [fftw3](http://www.fftw.org) library, which is the state
of the art on fft computation on a regular cpu. Even high-endg
mathematical programs like octave and matlab use fftw3.

This is where the idea of GPU_FFTW  originated. The idea is to have
**binary** compatibility with fftw3. The goal is to simply install
gpu_fftw and let your programs take advantage of the GPU automatically.

This is very hard to do, so there are some limitations, make sureg
you read the [limitations] section.

## Limitations

* *Float precission*: For now, Andrew's work only supports _float_ precission.
Most users typically use fftw3 with double precission. Hopefully
Andrew will add support for double precission to his work.

## TODO
- [ ] Fortran programs are not working, Fortran API needs to be added
- [ ] fftw_malloc family can be overriden to avoid copying arrays (Careful!)
- [ ] Test cases/benchmarks  for float squashing



