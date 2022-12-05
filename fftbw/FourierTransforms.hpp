/**
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
**/

#ifndef FOURIERTRANSFORMS_HPP
#define FOURIERTRANSFORMS_HPP

namespace FFTBW
{

    /**
        Offers static functions to compute the fast Fourier, sine and cosine transformation.
        The output will be written onto the input data and can be computed in-place.
    **/
    template <class FloatType>
    class FourierTransforms
    {
            /**
                FEATURES
                    - Optimize for accuracy:
                        If <look_up> is set to 'nullptr', a recursion relation is used to compute sin/cos,
                        which will produce numerical instabilities if the input size <N> is too large. To increase accuracy
                        pass a pointer to an array that stores the values 'sin(n*PI/<N>)' for n=0,1,...,N.

                    - Optimize for space:
                        If <in_place> is set to 'true', all operations will be done in-place by applying a bit-reversal permutation.
            **/

        public:
            FourierTransforms(){};
            virtual ~FourierTransforms() = default;



            /**
                FFT (complex)
                    - data size 'N' must be power of two with N>=2
                    - input/output format: 'complex[n] = data[2*n]+i*data[2*n+1]' -> N/2 complex numbers!
                    - inverse: 'sgn=+1' performs the forward transformation. If 'sgn=-1', computes the backwards transformation scaled by 'N/2'.
            **/
            static void fftComplex(
                FloatType * const   data,
                const int           N,
                const int           sgn,
                const FloatType *   look_up=nullptr,
                const bool          in_place=false);

            /**
                FFT (real)
                    - data size 'N' must be power of two with N>=2
                    - input: N real numbers
                    - output: N/2 complex numbers 'output[n] = data[2*n]+i*data[2*n+1]'. Exception: 'output[0]=data[0]+0*i' and 'output[N]=data[1]+0*i'
                    - inverse: 'sgn=+1' performs the forward transformation. If 'sgn=-1', computes the backwards transformation scaled by 'N/2'.
            **/
            static void fftReal(
                FloatType * const   data,
                const int           N,
                const int           sgn,
                const FloatType *   look_up=nullptr,
                const bool          in_place=false);

            /**
                DST-I
                    - data size 'N' must be power of two with N>=2
                    - input/output format: N real numbers with 'data[0]=0'
                    - inverse: The forward transformation is the backwards transformation scaled by 'N/2'.
            **/
            static void dst1(
                 FloatType * const  data,
                 const int          N,
                 const FloatType *  look_up=nullptr,
                 const bool         in_place=false);

            /**
                DCT-I
                    - data size must be given by 'N+1' with 'N=2**m' for m=0,1,2,...
                    - input/output format: N+1 real numbers
                    - inverse: The forward transformation is the backwards transformation scaled by 'N/2'.
            **/
            static void dct1(
                FloatType * const   data,
                const int           N,
                const FloatType *   look_up=nullptr,
                const bool          in_place=false);

            /**
                Cyclic Convolution
                    - data size 'N' must be power of two with N>=2
                    - 'A': Input signal (N real numbers)
                    - 'B': Impulse response scaled by N/2! (DFT of the kernel scaled by N/2).
                    - return: On exit, 'A' contains the cyclic, discrete convolution.
            **/
            static void convolve(
                FloatType * const A,
                const FloatType * B,
                const int N,
                const FloatType * look_up=nullptr,
                const bool in_place=false);

            /**
                Initialize a look-up table for improved stability in 'buffer' which must have size 'N/2+1' or larger.
            **/
            static void initLookUp(FloatType * buffer, const int N);

            static constexpr FloatType PI = FloatType((3.1415926535897932385L));


        private:
            // bit-reversal permutation of N/2 complex numbers for in-place computation called in fftComplexInPlace
            static void bitRevComplex(FloatType * const data, const int N);

            // in-place transformation of N/2 complex numbers with trigonometric recursion to compute the sine function on the fly
            static void fftComplexInPlace(FloatType * const data, const int N, const int sgn);

            // in-place transformation of N/2 complex numbers with look-up table of one period of sin(n*PI/N)
            static void fftComplexInPlace(FloatType * const data, const int N, const int sgn, const FloatType * look_up);

            // out-of-place transformation of N/2 complex numbers with with trigonometric recursion to compute the sine function on the fly
            static void fftComplexOutOfPlace(FloatType * const data, const int N, const int sgn);

            // out-of-place transformation of N/2 complex numbers with look-up table of one period of sin(n*PI/N)
            static void fftComplexOutOfPlace(FloatType * const data, const int N, const int sgn, const FloatType * look_up);

    };
}

#endif // FOURIERTRANSFORMS_HPP
