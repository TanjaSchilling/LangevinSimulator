/*
TensorUtils Version 0.1

Copyright 2022 Christoph Widder

This file is part of TensorUtils.

TensorUtils is free software: you can redistribute it and/or modify it under the terms of
the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

TensorUtils is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with TensorUtils.
If not, see <https://www.gnu.org/licenses/>.

*/

#include "TensorDerived.hpp"

#include "ErrorHandler.hpp"

using namespace TensorUtils;
using namespace ErrorHandler;

template<class T, int N>
TensorDerived<T,N>::TensorDerived() : TensorBase<T>()
{
    this->shape = std::vector<size_t>(N,0);
    this->incr = std::vector<size_t>(N,0);
};

template<class T, int N>
TensorDerived<T,N>::TensorDerived(const std::vector<size_t> shape) : TensorBase<T>(shape)
{
    if(N != shape.size())
    {
        throw RankMismatch("TensorUtils::TensorDerived<T,N>::TensorDerived:: Rank mismatch!");
    }
};

template<class T, int N>
TensorDerived<T,N>::TensorDerived(const std::vector<size_t> shape, const T& val) : TensorBase<T>(shape, val)
{
    if(N != shape.size())
    {
        throw RankMismatch("TensorUtils::TensorDerived<T,N>::TensorDerived:: Rank mismatch!");
    }
};

template<class T, int N>
void TensorDerived<T,N>::alloc(const std::vector<size_t> shape)
{
    if(N != shape.size())
    {
        throw RankMismatch("TensorUtils::TensorDerived<T,N>::alloc:: Rank mismatch!");
    }
    TensorBase<T>::alloc(shape);
};

template<class T, int N>
void TensorDerived<T,N>::alloc(const std::vector<size_t> shape, const T &val)
{
    if(N != shape.size())
    {
        throw RankMismatch("TensorUtils::TensorDerived<T,N>::alloc:: Rank mismatch!");
    }
    TensorBase<T>::alloc(shape, val);
};

template<class T, int N>
void TensorDerived<T,N>::clear()
{
    TensorBase<T>::clear();
    this->shape = std::vector<size_t>(N,0);
    this->incr = std::vector<size_t>(N,0);
}

template<class T, int N>
template<class T2>
TensorDerived<T,N>& TensorDerived<T,N>::operator=(const TensorBase<T2> &rhs)
{
    if(N != rhs.shape.size())
    {
        throw RankMismatch("TensorUtils::TensorDerived<T,N>::operator=:: Rank mismatch!");
    }
    TensorBase<T>::operator=(rhs);
    return *this;
};

template<class T>
template<class T2>
TensorDerived<T,-1>& TensorDerived<T,-1>::operator=(const TensorBase<T2> &rhs)
{
    TensorBase<T>::operator=(rhs);
    return *this;
};


template<class T, int N>
TensorDerived<T,N>& TensorDerived<T,N>::operator=(const std::vector<T> &rhs)
{
    TensorBase<T>::operator=(rhs);
    return *this;
};

template<class T>
TensorDerived<T,-1>& TensorDerived<T,-1>::operator=(const std::vector<T> &rhs)
{
    TensorBase<T>::operator=(rhs);
    return *this;
};

/**
    EXPLICIT TEMPLATE INSTANTIATION
**/

namespace TensorUtils
{
    #define INSTANTIATE_FUNCTION_TEMPLATES(X,Y,N) \
    template TensorDerived<X,N>& TensorDerived<X,N>::operator=<Y>(const TensorBase<Y> &rhs); \

    #define INSTANTIATE_FUNCTION_TEMPLATES1(X,Y) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,-1) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,0) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,1) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,2) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,3) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,4) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,5) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,6) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,7) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,Y,8) \

    #define INSTANTIATE(X) \
    template class TensorDerived<X,-1>; \
    template class TensorDerived<X,0>; \
    template class TensorDerived<X,1>; \
    template class TensorDerived<X,2>; \
    template class TensorDerived<X,3>; \
    template class TensorDerived<X,4>; \
    template class TensorDerived<X,5>; \
    template class TensorDerived<X,6>; \
    template class TensorDerived<X,7>; \
    template class TensorDerived<X,8>; \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,double) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,float) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,long double) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,unsigned char) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,signed char) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,unsigned short) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,short) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,unsigned) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,int) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,unsigned long) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,long) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,unsigned long long) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,long long) \

    #define INSTANTIATE_FLOATING_POINT_TYPES(X) \
    template class TensorDerived<X,-1>; \
    template class TensorDerived<X,0>; \
    template class TensorDerived<X,1>; \
    template class TensorDerived<X,2>; \
    template class TensorDerived<X,3>; \
    template class TensorDerived<X,4>; \
    template class TensorDerived<X,5>; \
    template class TensorDerived<X,6>; \
    template class TensorDerived<X,7>; \
    template class TensorDerived<X,8>; \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,double) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,float) \
    INSTANTIATE_FUNCTION_TEMPLATES1(X,long double) \

    #ifndef ENABLE_INTEGRAL_TYPES
    #define ENABLE_INTEGRAL_TYPES 1
    #endif // ENABLE_INTEGRAL_TYPES

    #if ENABLE_INTEGRAL_TYPES == 1
        INSTANTIATE(double)
        INSTANTIATE(float)
        INSTANTIATE(long double)
        INSTANTIATE(unsigned char)
        INSTANTIATE(signed char)
        INSTANTIATE(unsigned short)
        INSTANTIATE(short)
        INSTANTIATE(unsigned)
        INSTANTIATE(int)
        INSTANTIATE(unsigned long)
        INSTANTIATE(long)
        INSTANTIATE(unsigned long long)
        INSTANTIATE(long long)
    #else
        INSTANTIATE_FLOATING_POINT_TYPES(double)
        INSTANTIATE_FLOATING_POINT_TYPES(float)
        INSTANTIATE_FLOATING_POINT_TYPES(long double)
    #endif

    #undef INSTANTIATE_FUNCTION_TEMPLATES1
    #undef INSTANTIATE_FUNCTION_TEMPLATES
    #undef INSTANTIATE
    #undef INSTANTIATE_FLOATING_POINT_TYPES
}
