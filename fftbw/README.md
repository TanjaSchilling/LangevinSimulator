# fftbw

This project offers routines to compute the fast Fourier transform (FFT), 
the fast sine transform (DST-I) and the fast cosine transform (DCT-I) of real and complex data.
The number of elements must be a power of two. Supported data types are arrays of `float`, `double` or `long double`.
All routines are optimized for efficiency and can be done in-place.
Further, a lookup table for the sine function can be passed to all functions in order
to avoid recursion relations which might induce numerical inaccuracies for large data sizes.

Please see the header file for details on the usage.

Please report bugs to <christoph.widder[at]merkur.uni-freiburg.de>.

# License

MIT License

Copyright (c) 2022 Christoph Widder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
