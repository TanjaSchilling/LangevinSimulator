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

#include "FourierTransforms.hpp"

#include <cstring>
#include <cmath>
#include <algorithm>
#include <stdexcept>

/**
    PUBLIC FUNCTIONS
**/
template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::convolve(FloatType * const A, const FloatType * B, const int N, const FloatType * look_up, const bool in_place)
{
    fftReal(A,N,+1,look_up,in_place);
    A[0] = A[0]*B[0];
    A[1] = A[1]*B[1];
    double tmp;
    for(int n=2; n<N; n+=2)
    {
        A[n] = (tmp=A[n])*B[n]-A[n+1]*B[n+1];
        A[n+1] = tmp*B[n+1]+A[n+1]*B[n];
    }
    fftReal(A,N,-1,look_up,in_place);
}

template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::fftComplex(FloatType * const data, const int N, const int sgn, const FloatType * look_up, const bool in_place)
{
    if(N<2||N&(N-1))
    {
        throw std::invalid_argument(
 "\n\n In function: \n \
 FFTBW::FourierTransforms<FloatType>::fftComplex(FloatType * const data, const int N, const int sgn, const FloatType * look_up, const bool in_place). \n\n \
 Input size N must be given by N=2**m for some m>1. \n ");
    }
    if(sgn != 1 && sgn != -1)
    {
        throw std::invalid_argument(
 "\n\n In function: \n \
 FFTBW::FourierTransforms<FloatType>::fftComplex(FloatType * const data, const int N, const int sgn, const FloatType * look_up, const bool in_place). \n\n \
 The parameter <sgn> must be +1 or -1. \n ");
    }
    if(in_place)
    {
        if(look_up)
        {
            fftComplexInPlace(data,N,sgn,look_up);
        }
        else
        {
            fftComplexInPlace(data,N,sgn);
        }
    }
    else
    {
        if(look_up)
        {
            fftComplexOutOfPlace(data,N,sgn,look_up);
        }
        else
        {
            fftComplexOutOfPlace(data,N,sgn);
        }
    }
}

template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::fftReal(FloatType * const data, const int N, const int sgn, const FloatType * look_up, const bool in_place)
{
    if(N<2||N&(N-1))
    {
        throw std::invalid_argument(
 "\n\n In function: \n \
 FFTBW::FourierTransforms<FloatType>::fftReal(FloatType * const data, const int N, const int sgn, const FloatType * look_up, const bool in_place). \n\n \
 Input size N must be given by N=2**m for some m>1. \n ");
    }
    if(sgn != 1 && sgn != -1)
    {
        throw std::invalid_argument(
 "\n\n In function: \n \
 FFTBW::FourierTransforms<FloatType>::fftReal(FloatType * const data, const int N, const int sgn, const FloatType * look_up, const bool in_place). \n\n \
 The parameter <sgn> must be +1 or -1. \n ");
    }
    if(sgn==1)
    {
        fftComplex(data,N,+1,look_up,in_place);
    }
    FloatType tmp,tmp1r,tmp2r,tmp1i,tmp2i;
    data[0] = (tmp=data[0])+data[1];   // F_0
    data[1] = tmp-data[1];             // F_N
    if(look_up)
    {
        for(int n=2;n<=N/2;n+=2)
        {
            tmp1r=data[N-n];
            tmp1i=-data[N-n+1];
            tmp2r=data[n];
            tmp2i=data[n+1];
            tmp2r += tmp1r;
            tmp2i += tmp1i;
            tmp2r *= FloatType(0.5);
            tmp2i *= FloatType(0.5);
            tmp1r -=tmp2r;
            tmp1i -= tmp2i;
            tmp1r = -(tmp=tmp1r)*look_up[n]-sgn*tmp1i*look_up[N/2-n];
            tmp1i = sgn*tmp*look_up[N/2-n]-tmp1i*look_up[n];
            data[n] = tmp1r+tmp2r;
            data[n+1] = tmp1i+tmp2i;
            data[N-n] = tmp2r-tmp1r;
            data[N-n+1] = tmp1i-tmp2i;
        }
    }
    else
    {
        FloatType ctmp1r=std::cos(2*PI/N),ctmp1i=sgn*std::sin(2*PI/N),ctmp2r=0,ctmp2i=sgn;
        for(int n=2;n<=N/2;n+=2)
        {
            ctmp2r = (tmp=ctmp2r)*ctmp1r-ctmp2i*ctmp1i;
            ctmp2i = tmp*ctmp1i+ctmp2i*ctmp1r;
            tmp1r=data[N-n];
            tmp1i=-data[N-n+1];
            tmp2r=data[n];
            tmp2i=data[n+1];
            tmp2r += tmp1r;
            tmp2i += tmp1i;
            tmp2r *= FloatType(0.5);
            tmp2i *= FloatType(0.5);
            tmp1r -=tmp2r;
            tmp1i -= tmp2i;
            tmp1r = (tmp=tmp1r)*ctmp2r-tmp1i*ctmp2i;
            tmp1i = tmp*ctmp2i+tmp1i*ctmp2r;
            data[n] = tmp1r+tmp2r;
            data[n+1] = tmp1i+tmp2i;
            data[N-n] = tmp2r-tmp1r;
            data[N-n+1] = tmp1i-tmp2i;
        }
    }
    if(sgn==-1)
    {
        data[0]*=FloatType(0.5);
        data[1]*=FloatType(0.5);
        fftComplex(data,N,-1,look_up,in_place);
    }
}

template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::dst1(FloatType * const data, const int N, const FloatType * look_up, const bool in_place)
{
    if(N<2||N&(N-1))
    {
        throw std::invalid_argument(
 "\n\n In function: \n \
 FFTBW::FourierTransforms<FloatType>::dst1(FloatType *data, const int N, const FloatType * look_up, const bool in_place). \n\n \
 Input size N must be given by N=2**m for some m>1. \n ");
    }
    FloatType tmp,tmp2;
    data[0]=0;
    if(look_up)
    {
        for(int n=1;n<N/2;n++)
        {
            tmp = look_up[n]*(data[n]+data[N-n]);
            tmp2 = FloatType(0.5)*(data[n]-data[N-n]);
            data[n] = tmp+tmp2;
            data[N-n] = tmp-tmp2;
        }
    }
    else
    {
        FloatType ctmp1r=std::cos(PI/N),ctmp1i=std::sin(PI/N),ctmp2r=1,ctmp2i=0;
        for(int n=1;n<N/2;n++)
        {
            ctmp2r = (tmp=ctmp2r)*ctmp1r-ctmp2i*ctmp1i;
            ctmp2i = tmp*ctmp1i+ctmp2i*ctmp1r;
            tmp = ctmp2i*(data[n]+data[N-n]);
            tmp2 = FloatType(0.5)*(data[n]-data[N-n]);
            data[n] = tmp+tmp2;
            data[N-n] = tmp-tmp2;
        }
    }
    data[N/2] *= 2;
    fftReal(data,N,1,look_up,in_place);
    data[1] = FloatType(0.5)*data[0];
    data[0] = 0;
    for(int n=2; n+1<N;n+=2)
    {
        tmp = data[n];
        data[n] = data[n+1];
        data[n+1] = data[n-1]+tmp;
    }
}

template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::dct1(FloatType *data, const int N, const FloatType * look_up, const bool in_place)
{
    if(N<1||N&(N-1))
    {
        throw std::invalid_argument(
 "\n\n In function: \n \
 FFTBW::FourierTransforms<FloatType>::dct1(FloatType *data, const int N, const FloatType * look_up, const bool in_place). \n\n \
 Input size (N+1) must be given by N=2**m for some m>0. \n ");
    }
    int n;
    FloatType tmp,tmp2,f1;
    f1 = FloatType(0.5)*(data[0]-data[N]);

    if(look_up)
    {
        for(n=1;n<N/2;n++)
        {
            f1 += data[n]*look_up[N/2-n];
            f1 -= data[N-n]*look_up[N/2-n];
            tmp = look_up[n]*(data[n]-data[N-n]);
            tmp2 = FloatType(0.5)*(data[n]+data[N-n]);
            data[n] = tmp2-tmp;
            data[N-n] = tmp2+tmp;
        }
    }
    else
    {
        FloatType ctmp1r=std::cos(PI/N),ctmp1i=std::sin(PI/N),ctmp2r=1,ctmp2i=0;
        for(n=1;n<N/2;n++)
        {
            ctmp2r = (tmp=ctmp2r)*ctmp1r-ctmp2i*ctmp1i;
            ctmp2i = tmp*ctmp1i+ctmp2i*ctmp1r;
            f1 += data[n]*ctmp2r;
            f1 -= data[N-n]*ctmp2r;
            tmp = ctmp2i*(data[n]-data[N-n]);
            tmp2 = FloatType(0.5)*(data[n]+data[N-n]);
            data[n] = tmp2-tmp;
            data[N-n] = tmp2+tmp;
        }
    }
    data[0]=FloatType(0.5)*(data[0]+data[N]);
    if(N>1)
    {
        fftReal(data,N,1,look_up,in_place);
    }
    data[N]=data[1];
    data[1] = f1;
    for(n=3;n<N;n+=2)
    {
        data[n] = data[n-2]+data[n];
    }
}

/**
    Initialize a look-up table for improved stability in 'buffer' which must have size 'N/2+1' or larger.
**/
template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::initLookUp(FloatType * buffer, const int N)
{
    for(int n=0;n<=N/2;n++)
    {
        buffer[n] = std::sin(n*PI/N);
    }
}

/**
    PRIVATE FUNCTIONS
**/

// bit-reversal permutation of complex numbers for in-place computation called in fftComplexInPlace
template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::bitRevComplex(FloatType * const data, const int N)
{
    int i,j,m,n;
    int n_bits = static_cast<int>(0.5+std::log2(N/2));
    for(i=0;i<N/2;i++)
    {
        j=0,m=1,n=i;
        if(n&1)
        {
            j++;
        }
        while(m<n_bits)
        {
            j<<=1;
            if( (n>>=1) & 1 )
            {
                j++;
            }
            m++;
        }
        if(j<i)
        {
            std::swap(data[2*j],data[2*i]);
            std::swap(data[2*j+1],data[2*i+1]);
        }
    }
}

// in-place transformation of N/2 complex numbers with trigonometric recursion to compute the sine function on the fly
template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::fftComplexInPlace(FloatType * const data, const int N, const int sgn)
{
    bitRevComplex(data,N);
    int i,j,m=1;
    FloatType tmpr,tmpi,ctmp1r,ctmp1i,ctmp2r,ctmp2i;
    while(m<N/2)
    {
        ctmp1r=std::cos(PI/m);
        ctmp1i=std::sin(sgn*PI/m);
        m*=2;
        for(i=0;i<N;i+=2*m)
        {
            ctmp2r=1;
            ctmp2i=0;
            for(j=i;j<m+i;j+=2)
            {
                tmpr = data[j+m]*ctmp2r-data[j+m+1]*ctmp2i;
                tmpi = data[j+m]*ctmp2i+data[j+m+1]*ctmp2r;
                data[j+m] = data[j]-tmpr;
                data[j+m+1] = data[j+1]-tmpi;
                data[j] += tmpr;
                data[j+1] += tmpi;
                ctmp2r = (tmpr=ctmp2r)*ctmp1r-ctmp2i*ctmp1i;
                ctmp2i = tmpr*ctmp1i+ctmp2i*ctmp1r;
            }
        }
    }
}

// in-place transformation of N/2 complex numbers with look-up table of one period of sin(n*PI/N)
template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::fftComplexInPlace(FloatType * const data, const int N, const int sgn, const FloatType * look_up)
{
    bitRevComplex(data,N);
    int i,j,m=1;
    FloatType tmpr,tmpi;
    int index;
    while(m<N/2)
    {
        m*=2;
        for(i=0;i<N;i+=2*m)
        {
            for(j=i;j<m/2+i;j+=2)
            {
                index = (N/m)*(j-i);
                tmpr = data[j+m]*look_up[N/2-index]-sgn*data[j+m+1]*look_up[index];
                tmpi = sgn*data[j+m]*look_up[index]+data[j+m+1]*look_up[N/2-index];
                data[j+m] = data[j]-tmpr;
                data[j+m+1] = data[j+1]-tmpi;
                data[j] += tmpr;
                data[j+1] += tmpi;
            }
            for(;j<m+i;j+=2)
            {
                index = (N/m)*(j-i);
                tmpr = -data[j+m]*look_up[index-N/2]-sgn*data[j+m+1]*look_up[N-index];
                tmpi = sgn*data[j+m]*look_up[N-index]-data[j+m+1]*look_up[index-N/2];
                data[j+m] = data[j]-tmpr;
                data[j+m+1] = data[j+1]-tmpi;
                data[j] += tmpr;
                data[j+1] += tmpi;
            }
        }
    }
}

// out-of-place transformation of N/2 complex numbers with with trigonometric recursion to compute the sine function on the fly
template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::fftComplexOutOfPlace(FloatType * const data, const int N, const int sgn)
{
    int i,j,n=N/2;
    FloatType * const buffer = new FloatType[N];
    FloatType * ptr1 = data;
    FloatType * ptr2 = buffer;
    FloatType tmpr,tmpi,ctmp1r,ctmp1i,ctmp2r,ctmp2i;
    while(n>1)
    {
        ctmp1r=std::cos(2*n*PI/N);
        ctmp1i=sgn*std::sin(2*n*PI/N);
        ctmp2r=1;
        ctmp2i=0;
        for(j=0;j<N/2;j+=n)
        {
            for(i=2*j;i<n+2*j;i+=2)
            {
                tmpr = ptr1[i+n]*ctmp2r-ptr1[i+n+1]*ctmp2i;
                tmpi = ptr1[i+n]*ctmp2i+ptr1[i+n+1]*ctmp2r;
                ptr2[i-j+N/2] = ptr1[i]-tmpr;
                ptr2[i-j+N/2+1] = ptr1[i+1]-tmpi;
                ptr2[i-j] = ptr1[i]+tmpr;
                ptr2[i-j+1] = ptr1[i+1]+tmpi;
            }
            ctmp2r = (tmpr=ctmp2r)*ctmp1r-ctmp2i*ctmp1i;
            ctmp2i = tmpr*ctmp1i+ctmp2i*ctmp1r;
        }
        std::swap(ptr1,ptr2);
        n/=2;
    }
    if(data!=ptr1)
    {
        std::memcpy(data, ptr1, sizeof(FloatType)*N );
    }
    delete [] buffer;
}

// out-of-place transformation of N/2 complex numbers with look-up table of one period of sin(n*PI/N)
template <class FloatType>
void FFTBW::FourierTransforms<FloatType>::fftComplexOutOfPlace(FloatType * const data, const int N, const int sgn, const FloatType * look_up)
{
    int i,j,n=N/2;
    FloatType * const buffer = new FloatType[N];
    FloatType * ptr1 = data;
    FloatType * ptr2 = buffer;
    FloatType tmpr,tmpi;
    while(n>1)
    {
        for(j=0;j<N/4;j+=n)
        {
            for(i=2*j;i<n+2*j;i+=2)
            {
                tmpr = ptr1[i+n]*look_up[N/2-2*j]-sgn*ptr1[i+n+1]*look_up[2*j];
                tmpi = sgn*ptr1[i+n]*look_up[2*j]+ptr1[i+n+1]*look_up[N/2-2*j];
                ptr2[i-j+N/2] = ptr1[i]-tmpr;
                ptr2[i-j+N/2+1] = ptr1[i+1]-tmpi;
                ptr2[i-j] = ptr1[i]+tmpr;
                ptr2[i-j+1] = ptr1[i+1]+tmpi;
            }
        }
        for(;j<N/2;j+=n)
        {
            for(i=2*j;i<n+2*j;i+=2)
            {
                tmpr = -ptr1[i+n]*look_up[2*j-N/2]-sgn*ptr1[i+n+1]*look_up[N-2*j];
                tmpi = sgn*ptr1[i+n]*look_up[N-2*j]-ptr1[i+n+1]*look_up[2*j-N/2];
                ptr2[i-j+N/2] = ptr1[i]-tmpr;
                ptr2[i-j+N/2+1] = ptr1[i+1]-tmpi;
                ptr2[i-j] = ptr1[i]+tmpr;
                ptr2[i-j+1] = ptr1[i+1]+tmpi;
            }
        }
        std::swap(ptr1,ptr2);
        n/=2;
    }
    if(data!=ptr1)
    {
        std::memcpy(data, ptr1, sizeof(FloatType)*N );
    }
    delete [] buffer;
}

template class FFTBW::FourierTransforms<float>;
template class FFTBW::FourierTransforms<double>;
template class FFTBW::FourierTransforms<long double>;

