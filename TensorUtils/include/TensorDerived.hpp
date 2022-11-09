/**
\internal
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
\endinternal
**/

#ifndef TENSORDERIVED_HPP
#define TENSORDERIVED_HPP

#include "TensorBase.hpp"

namespace TensorUtils
{
    /*!
        \addtogroup TensorUtils
        @{
    */

    //! This class defines a tensor with fixed rank N=0,1,... and inherits from \ref TensorBase. The spezialization for N=-1 defines a tensor with mutable rank.
    /*!
        This class inherits all functionality from the base class \ref TensorBase.
        It allows to separate the types for tensors of different ranks, if desired.
        This is in particular useful to overload functions for different ranks of its arguments.
        If any method would change the rank for N>=0, \ref ErrorHandler::RankMismatch is thrown.
        \code
        #include "TensorUtils.hpp"

        int main()
        {
            TensorUtils::tensor<double,3> bar;

            bool i_am_true = (bar.empty() && bar.shape.size() == 3 && bar.incr.size()==3);

            TensorUtils::tensor<float,4> foo({2,3,5,7}, 1.0);

            TensorUtils::tensor<long double> foobar({2,3,5,7,11}, 2.0);

            // foo = foobar;            // throws RankMismatch!

            // foo.alloc(bar.shape);    // throws RankMismatch!

            foobar = foo;               // OK!

            return 0;
        }
        \endcode
    */
    template<class T, int N>
    class TensorDerived : public TensorBase<T>
    {
        public:
            //! Constructor is inherited from \ref TensorBase and resizes \ref shape and \ref incr with size N.
            TensorDerived();

            //! Inherits from \ref TensorBase and throws \ref ErrorHandler::RankMismatch if shape.size()!=N.
            TensorDerived(const std::vector<size_t> shape);

            //! Inherits from \ref TensorBase and throws \ref ErrorHandler::RankMismatch if shape.size()!=N.
            TensorDerived(const std::vector<size_t> shape, const T& val);

            //! Inherits from \ref TensorBase and throws \ref ErrorHandler::RankMismatch if shape.size()!=N.
            void alloc(const std::vector<size_t> shape);

            //! Inherits from \ref TensorBase and throws \ref ErrorHandler::RankMismatch if shape.size()!=N.
            void alloc(const std::vector<size_t> shape, const T &val);

            //! Inherits from \ref TensorBase and resizes \ref shape and \ref incr with size N.
            void clear();

            //! Calls \ref TensorBase<T>::operator= and returns *this by reference. Throws \ref ErrorHandler::RankMismatch if shape.size()!=N.
            template<class T2> TensorDerived<T,N>& operator= (const TensorBase<T2> &rhs);

            //! Calls \ref TensorBase<T>::operator= and returns *this by reference. Throws \ref ErrorHandler::RankMismatch if shape.size()!=N.
            TensorDerived<T,N>& operator= (const std::vector<T> &rhs);
    };

    //! This class specialization defines a tensor with mutable rank and inherits from \ref TensorBase.
    template<class T>
    class TensorDerived<T,-1> : public TensorBase<T>
    {
        public:
            //! Constructor is inherited from \ref TensorBase.
            TensorDerived() : TensorBase<T>() {};

            //! Constructor is inherited from \ref TensorBase.
            TensorDerived(const std::vector<size_t> shape)  : TensorBase<T>(shape) {};

            //! Constructor is inherited from \ref TensorBase.
            TensorDerived(const std::vector<size_t> shape, const T& val) : TensorBase<T>(shape, val) {};

            //! Calls \ref TensorBase<T>::operator= and returns *this by reference.
            template<class T2> TensorDerived<T,-1>& operator= (const TensorBase<T2> &rhs);

            //! Calls \ref TensorBase<T>::operator= and returns *this by reference.
            TensorDerived<T,-1>& operator= (const std::vector<T> &rhs);
    };
    /*! @} */
}

#endif // TENSORDERIVED_HPP

