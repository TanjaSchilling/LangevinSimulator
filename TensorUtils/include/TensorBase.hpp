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

#ifndef TENSORBASE_HPP
#define TENSORBASE_HPP

#include <vector>
#include <string>

namespace TensorUtils
{
    /*!
        \brief This is the main class of this project.
        It inherits from std::vector<T> and adds methods to make it a tensor.
    */
    template<class T>
    class TensorBase : public std::vector<T>
    {
        public:

            // avoid name hiding of base class constructors
            using std::vector<T>::vector;

            /*!
                Empty constructor.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> foo;

                    return 0;
                }
                \endcode
            */
            TensorBase();

            /*!
                Constructor. Calls \ref alloc(const std::vector<size_t> &).
                \param shape    Specifies the number of indices and their ranges.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    // TensorUtils::tensor<double> foo({}); // invalid syntax: ambiguity with copy and move constructor!

                    TensorUtils::tensor<double> foo({2,3,5,7});

                    return 0;
                }
                \endcode
            */
            TensorBase(const std::vector<size_t> &shape);

            /*!
                Constructor. Calls \ref alloc(const std::vector<size_t> &, const T&).
                \param shape    Specifies the number of indices and their ranges.
                \param val      All components are initialized with this value.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<float> foo({}, 1.0); // scalar
                    TensorUtils::tensor<float> bar({2,3,5,7}, 1.0);

                    return 0;
                }
                \endcode
            */
            TensorBase(const std::vector<size_t> &shape, const T& val);

            //! \private
            virtual ~TensorBase();

            /*!
                Allocates the necessary memory and initializes \ref shape and \ref incr accordingly.
                If an empty shape is received, the tensor is a scalar with exactly one component.
                \param shape    Used to initialize \ref shape.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<float> foo;

                    foo.alloc({2,3,5,7});

                    return 0;
                }
                \endcode
            */
            void alloc(const std::vector<size_t> &shape);

            /*!
                Allocate memory and initialize all components.
                Calls \ref alloc(const std::vector<size_t> &shape) and \ref init(const T& val).
                \param shape    Used to initialize \ref shape.
                \param val      All components are initialized with this value.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<float> foo;

                    foo.alloc({2,3,5,7}, 1.0);

                    return 0;
                }
                \endcode
            */
            void alloc(const std::vector<size_t> &shape, const T& val);

            /*!
                \param val All components are initialized with this value.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> foo({2,3,5,7});

                    long double some_value = 1.0L;

                    foo.init(some_value);

                    return 0;
                }
                \endcode
            */
            void init(const T& val);

            /*!
                Initialize all components with lexicographical enumeration.
                \param val Value of first component.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> my_tensor({10,10});

                    my_tensor.arange(1); // same as the following

                    my_tensor(0,0)=1;
                    my_tensor(0,1)=2;
                    // ...
                    my_tensor(9,9)=100;

                    return 0;
                }
                \endcode
            */
            void arange(T val=0);

            /*!
                Clears the memory and the member variables \ref shape and \ref incr.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<float> foo({2,3,5,7}, 1.0);

                    foo.clear();

                    return 0;
                }
                \endcode
            */
            void clear();

            /*!
                Prints all sub-matrices in lexicographical order to "std::cout".
                Vectors are printed as row-vectors.
                The format is the same as for \ref write.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<float> foo({2,3,5,7});

                    foo.arange();

                    foo.print();

                    return 0;
                }
                \endcode
            */
            void print();

            /*!
                Reads arbitrary tensors from text or binary files.
                \param path Specifies the source path.
                    The extension specifies the file format.
                    For text files use any extension except the following that are used for binary files:
                        - .f32  float
                        - .f64  double
                        - .f80  long double
                        - .uc   unsigned char
                        - .sc   signed char
                        - .us   unsigned short
                        - .s    short
                        - .u    unsigned
                        - .int  int
                        - .ul   unsigned long
                        - .l    long
                        - .ull  unsigned long long
                        - .ll   long long

                For text files, the first line must contain the shape of the tensor. Empty lines are ignored.
                The header line is followed by a lexicographical list of all sub-matrices. Vectors are row-vectors.
                Note that \ref print will display the same format.

                Binary files are formatted as follows.
                The first block contains sizeof(size_t) bytes specifying shape.size().
                The second block contains shape.size()*sizeof(size_t) bytes specifying the components of \ref shape.
                The third block contains sizeof(size_t) bytes specifying the container size.
                The fourth block contains this->size()*sizeof(T) bytes specifying the components of the tensor,
                where T is the type of the components specified by the extension.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    using namespace TensorUtils;
                    using namespace ErrorHandler;

                    tensor<double> foo;
                    try
                    {
                        foo.read("foo.txt");
                        foo.read("foo.f32");
                        foo.read("foo.ull");
                    }
                    catch(UnableToOpenFile &ex) // unable to open file
                    {
                        //
                    }
                    catch(ShapeMismatch &ex) // Shape in header does not match given data: corrupted file?
                    {
                        //
                    }
                    catch(std::exception &ex) // catch any other exception
                    {
                        //
                    }

                    return 0;
                }
                \endcode
            */
            void read(std::string path);

            /*!
                Write arbitrary tensors to text or binary files.
                \param oname    Specifies the file name.
                    The extension specifies the file format.
                    For text files use any extension except the following that are used for binary files:
                        - .f32  float
                        - .f64  double
                        - .f80  long double
                        - .uc   unsigned char
                        - .sc   signed char
                        - .us   unsigned short
                        - .s    short
                        - .u    unsigned
                        - .int  int
                        - .ul   unsigned long
                        - .l    long
                        - .ull  unsigned long long
                        - .ll   long long
                \param folder   Specifies the output path.

                See \ref read for details on the file format.
                You may add the number of significant digits when writing text files,
                see \ref write(std::string,std::string,int).
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    using namespace TensorUtils;

                    tensor<double> foo({2,3,5,7}, 1.0);

                    foo.write("foo.txt", ".", 10);  // text file: writes 10 significant digits

                    foo.write("foo.dat", "./"); // text file: if floating point: uses std::numeric_limits<T>::max_digits10

                    foo.write("foo.f32", ".");  // binary file: float
                    foo.write("foo.ull", ".");  // binary file: unsigned long long

                    return 0;
                }
                \endcode
            */
            void write(std::string oname, std::string folder);

            /*!
                For text files only. See also \ref write(std::string,std::string) for details.
                \param oname      Specifies the file name.
                \param folder     Specifies the output path.
                \param precision  Number of significant digits when writing text files for floating point types.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    using namespace TensorUtils;

                    tensor<double> foo({2,3,5,7}, 1.0);

                    try
                    {
                        foo.write("foo.txt", ".", 10); // OK!
                        foo.write("foo.f32", ".", 10); // throws an error!
                    }
                    catch(std::runtime_error &ex) // Binary file extension but text file requested!
                    {
                        return 1;
                    }

                    return 0;
                }
                \endcode
            */
            void write(std::string oname, std::string folder, int precision);

            /*!
                Permutes the indices of the tensor and returns by value.
                \param axes Permutation of (0,1,...,N-1), where N is the rank. Indices are transposed accordingly.
                \return Tensor with transposed indices.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> foo({2,3,5,7});
                    foo.arange();
                    foo = foo.transpose({0,2,1,3}); // New shape is: {2,5,3,7}

                    return 0;
                }
                \endcode
            */
            TensorBase<T> transpose(const std::vector<unsigned> &axes);

            /*!
                Slices a sub-tensor and returns by value.
                \param idx_at Permutation of (0,1,...,N-1), where N is the rank. Indices are transposed accordingly.
                \return Returns a the sub-tensor addressed by \p idx_at by value.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> foo({2,3,5,7});
                    foo.arange();

                    TensorUtils::tensor<double> bar;
                    bar = foo.slice({1,2}); // contains the last sub-tensor with shape {5,7}

                    bar.print();

                    return 0;
                }
                \endcode
            */
            TensorBase<T> slice(const std::vector<size_t> &idx_at);

            /*!
                Assigns a new \ref shape to this tensor and updates \ref incr.
                \param shape Specifies the new shape.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> foo({2,3,5,7});
                    foo.arange();

                    foo.reshape({2*3,5*7}); // same components with same order, but different shape

                    return 0;
                }
                \endcode
            */
            TensorBase<T>& reshape(const std::vector<size_t> &shape);

            /*!
                Returns a generalized tensor product by value.
                Indices are represented by signed integers.
                The parameters \p idx_lhs and \p idx_rhs specify the indices for the two operands.
                Negative integers are summed over.
                Multiple occurrences of the same index performs element-wise multiplication (Hadamard product).
                Distinct indices perform the usual tensor product.
                It is possible to mix summation, element-wise multiplication and the usual tensor product as desired.
                The order of the return value can be set as desired and is given in increasing order of the resulting indices.
                Additionally, it is possible to compute only a sub-tensor of the final result by setting the parameter \p idx_at.
                \param rhs Second operand.
                \param idx_lhs Indices of first operand represented by signed intergers.
                \param idx_rhs Indices of second operand represented by signed integers.
                \param idx_at  Indices specifying the sub-tensor to be computed.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> X({2,3,5,7},1);
                    TensorUtils::tensor<double> Y({2,3,5,7},2);
                    TensorUtils::tensor<double> Z;

                    Z = X.dot(Y,{-1,-2,-3,-4},{-1,-2,-3,-4});   // full contraction: Z is a scalar!

                    Z = X.dot(Y,{1,2,3,4},{1,2,3,4});           // Hadamard product: Z has shape {2,3,5,7}

                    Z = X.dot(Y,{1,2,3,4},{8,7,6,5});           // tensor product: Z has shape {2,3,5,7,7,5,3,2}

                    Z = X.dot(Y,{1,2,3,4},{5,6,7,8}, {1,2,4,6}); // compute sub-tensor of tensor-product

                    X.alloc({2,3,7,7},1);
                    Y.alloc({7,5,3,11},2);

                    Z = X.dot(Y,{3,2,-5,-5},{-5,4,2,1});    // generalized tensor product: Z has shape {11,3,2,5}

                    return 0;
                }
                \endcode
            */
            template<class T2>
            TensorBase<T> dot(
                TensorBase<T2>&             rhs,
                const std::vector<int>      &idx_lhs,
                const std::vector<int>      &idx_rhs,
                const std::vector<size_t>   &idx_at={});

            /*!
                Sum over specified axes and return the remaining subtensor.
                Indices are represented by signed integers.
                The parameters \p idx_lhs enumerates the indices of this tensor.
                Indices represented by negative integers are summed over.
                The order of the return value can be set as desired and is given in increasing order of the resulting indices.
                Optionally, it is possible to compute only a sub-tensor of the final result by setting the parameter \p idx_at.
                \param idx_lhs Indices of first operand represented by signed integers.
                \param idx_at  Indices specifying the sub-tensor to be computed.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> X({3,3,5,5},1);
                    TensorUtils::tensor<double> Z;

                    Z = X.contract({-1,-2,-3,-4});   // sum of all components!

                    Z = X.contract({1,2,-1,-1}); // trace of all submatrices

                    Z = X.contract({2,1,-1,-1}); // same but transposed

                    Z = X.contract({1,1,-1,-1}); // main diagonal of the previous result

                    Z = X.contract({1,2,-1,-1}, {0,0}); // trace of first submatrix

                    return 0;
                }
                \endcode
            */
            TensorBase<T> contract(const std::vector<int> &idx_lhs, const std::vector<size_t> &idx_at={});

            /*!
                Assigns the components in lexicographical order from a vector.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7});

                    foo = std::vector<long double>(foo.size(), 1.0L); // initialize from a vector

                    return 0;
                }
                \endcode
            */
            TensorBase<T>& operator= (const std::vector<T>& rhs);

            /*!
                Assigns this tensor with \p rhs. If the components have the same type, the default copy assignment is invoked.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar;

                    bar = foo;

                    return 0;
                }
                \endcode
            */
            template<class T2> TensorBase<T>&   operator=   (const TensorBase<T2>& rhs);

            /*!
                Add the tensor \p rhs. Number of components must match, else \ref ErrorHandler::ShapeMismatch is thrown.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar({2,3,5,7},1.0);

                    foo += bar;

                    bar.alloc({2*3,5*7},1.0);

                    foo += bar;

                    return 0;
                }
                \endcode
            */
            template<class T2> TensorBase<T>&   operator+=  (const TensorBase<T2>& rhs);

            /*!
                Returns the sum of this tensor \p with rhs. Number of components must match, else \ref ErrorHandler::ShapeMismatch is thrown.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar({2,3,5,7},1.0);

                    foo = foo + bar;

                    bar.alloc({2*3,5*7},1.0);

                    foo = foo + bar;

                    return 0;
                }
                \endcode
            */
            template<class T2> TensorBase<T>    operator+   (const TensorBase<T2>& rhs);

            /*!
                Substract the tensor \p rhs. Number of components must match, else \ref ErrorHandler::ShapeMismatch is thrown.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar({2,3,5,7},1.0);

                    foo -= bar;

                    bar.alloc({2*3,5*7},1.0);

                    foo -= bar;

                    return 0;
                }
                \endcode
            */
            template<class T2> TensorBase<T>&   operator-=  (const TensorBase<T2>& rhs);

            /*!
                Returns the difference of this tensor \p with rhs. Number of components must match, else \ref ErrorHandler::ShapeMismatch is thrown.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar({2,3,5,7},1.0);

                    foo = foo - bar;

                    bar.alloc({2*3,5*7},1.0);

                    foo = foo - bar;

                    return 0;
                }
                \endcode
            */
            template<class T2> TensorBase<T>    operator-   (const TensorBase<T2>& rhs);

            /*!
                Multiply this tensor with \p rhs.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);

                    foo *= 2;

                    return 0;
                }
                \endcode
            */
            TensorBase<T>&                      operator*=  (const T& rhs);

            /*!
                Scalar multiplication from the right.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);

                    foo = foo*2;

                    return 0;
                }
                \endcode
            */
            TensorBase<T>                       operator*   (const T& rhs);

            /*!
                Divide this tensor with \p rhs.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);

                    foo /= 2;

                    return 0;
                }
                \endcode
            */
            TensorBase<T>&                      operator/=  (const T& rhs);

            /*!
                Element-wise division.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);

                    foo = foo/2;

                    return 0;
                }
                \endcode
            */
            TensorBase<T>                       operator/   (const T& rhs);

            /*!
                Initialize this tensor from an array in lexicographical order. No error-handling!
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7});
                    foo.arange();

                    long double raw_data[2*3*5*7];
                    foo >> raw_data[0]; // copy data to array
                    foo << raw_data[0]; // initialize from array

                    long double multi_array[2][3][5][7];
                    foo >> multi_array[0][0][0][0]; // copy data to multi-dimensional array
                    foo << multi_array[0][0][0][0]; // initialize from multi-dimensional array

                    return 0;
                }
                \endcode
            */
            template<class T2> TensorBase<T>&   operator<<  (T2& rhs);

            /*!
                Copy the components in lexicographical order to an array. No error-handling!
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7});
                    foo.arange();

                    long double raw_data[2*3*5*7];
                    foo >> raw_data[0]; // copy data to array
                    foo << raw_data[0]; // initialize from array

                    long double multi_array[2][3][5][7];
                    foo >> multi_array[0][0][0][0]; // copy data to multi-dimensional array
                    foo << multi_array[0][0][0][0]; // initialize from multi-dimensional array

                    return 0;
                }
                \endcode
            */
            template<class T2>  T2&             operator>>  (T2& rhs);

            /*!
                Scalar multiplication from the left.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);

                    foo = 2*foo;

                    return 0;
                }
                \endcode
            */
            friend TensorBase<T> operator* (const T& lhs, TensorBase<T> rhs) { rhs*=lhs; return rhs; };

            /*!
                Assign a sub-tensor this tensor with a sub-tensor of \p rhs. Number of components must match, else \ref ErrorHandler::ShapeMismatch is thrown.
                \param rhs Second operand.
                \param at_lhs Indices specifying the sub-tensor of the first operand.
                \param at_rhs Indices specifying the sub-tensor of the second operand.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar({2*3,5,7},1.0);

                    foo.assign(bar, {1,2}, {5});

                    return 0;
                }
                \endcode
            */
            template<class T2>
            TensorBase<T>& assign(
                TensorBase<T2>              &rhs,
                const std::vector<size_t>   &at_lhs={},
                const std::vector<size_t>   &at_rhs={});

            /*!
                Add a sub-tensor of \p rhs to a sub-tensor of this tensor. Number of components must match, else \ref ErrorHandler::ShapeMismatch is thrown.
                \param rhs Second operand.
                \param at_lhs Indices specifying the sub-tensor of the first operand.
                \param at_rhs Indices specifying the sub-tensor of the second operand.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar({2*3,5,7},1.0);

                    foo.add(bar, {1,2}, {5});

                    return 0;
                }
                \endcode
            */
            template<class T2>
            TensorBase<T>& add(
                TensorBase<T2>              &rhs,
                const std::vector<size_t>   &at_lhs={},
                const std::vector<size_t>   &at_rhs={});

            /*!
                Substract a sub-tensor of \p rhs from a sub-tensor of this tensor. Number of components must match, else \ref ErrorHandler::ShapeMismatch is thrown.
                \param rhs Second operand.
                \param at_lhs Indices specifying the sub-tensor of the first operand.
                \param at_rhs Indices specifying the sub-tensor of the second operand.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar({2*3,5,7},1.0);

                    foo.substract(bar, {1,2}, {5});

                    return 0;
                }
                \endcode
            */
            template<class T2>
            TensorBase<T>& substract(
                TensorBase<T2>              &rhs,
                const std::vector<size_t>   &at_lhs={},
                const std::vector<size_t>   &at_rhs={});

            /*!
                Multiply a sub-tensor of this tensor with \p rhs.
                \param rhs Second operand.
                \param at_lhs Indices specifying the sub-tensor of the first operand.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);

                    foo.multiply(5, {1,2});

                    return 0;
                }
                \endcode
            */
            TensorBase<T>& multiply(
                const T                     &rhs,
                const std::vector<size_t>   &at_lhs={});

            /*!
                Divide a sub-tensor of this tensor with \p rhs.
                \param rhs Second operand.
                \param at_lhs Indices specifying the sub-tensor of the first operand.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);

                    foo.divide(5, {1,2});

                    return 0;
                }
                \endcode
            */
            TensorBase<T>& divide(
                const T                     &rhs,
                const std::vector<size_t>   &at_lhs={});

            /*!
                Return the sum of a sub-tensor of this tensor with a sub-tensor of \p rhs.
                Number of components must match, else \ref ErrorHandler::ShapeMismatch is thrown.
                \param rhs Second operand.
                \param at_lhs Indices specifying the sub-tensor of the first operand.
                \param at_rhs Indices specifying the sub-tensor of the second operand.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar({2*3,5,7},1.0);
                    TensorUtils::tensor<double> foobar;

                    foobar = foo.plus(bar, {1,2}, {5}); // foobar has shape {5,7}

                    return 0;
                }
                \endcode
            */
            template<class T2>
            TensorBase<T> plus(
                TensorBase<T2>              &rhs,
                const std::vector<size_t>   &at_lhs={},
                const std::vector<size_t>   &at_rhs={});

            /*!
                Return the difference of a sub-tensor of this tensor with a sub-tensor of \p rhs.
                Number of components must match, else \ref ErrorHandler::ShapeMismatch is thrown.
                \param rhs Second operand.
                \param at_lhs Indices specifying the sub-tensor of the first operand.
                \param at_rhs Indices specifying the sub-tensor of the second operand.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar({2*3,5,7},1.0);
                    TensorUtils::tensor<double> foobar;

                    foobar = foo.minus(bar, {1,2}, {5}); // foobar has shape {5,7}

                    return 0;
                }
                \endcode
            */
            template<class T2>
            TensorBase<T> minus(
                TensorBase<T2>              &rhs,
                const std::vector<size_t>   &at_lhs={},
                const std::vector<size_t>   &at_rhs={});

            /*!
                Return the product of a sub-tensor of this tensor with \p rhs.
                \param rhs Second operand.
                \param at_lhs Indices specifying the sub-tensor of the first operand.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar;

                    bar = foo.product(5, {1,2}); // bar has shape {5,7}

                    return 0;
                }
                \endcode
            */
            TensorBase<T> product(
                const T                     &rhs,
                const std::vector<size_t>   &at_lhs={});

            /*!
                Return the quotient of a sub-tensor of this tensor with \p rhs.
                \param rhs Second operand.
                \param at_lhs Indices specifying the sub-tensor of the first operand.

                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);
                    TensorUtils::tensor<float> bar;

                    bar = foo.quotient(5, {1,2}); // bar has shape {5,7}

                    return 0;
                }
                \endcode
            */
            TensorBase<T> quotient(
                const T                     &rhs,
                const std::vector<size_t>   &at_lhs={});

            /*!
                Access a component or the first component of a sub-tensor.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<long double> foo({2,3,5,7},1.0);

                    int elem = 0;
                    for(size_t n0=0; n0<foo.shape[0]; n0++)
                    {
                        for(size_t n1=0; n1<foo.shape[1]; n1++)
                        {
                            for(size_t n2=0; n2<foo.shape[2]; n2++)
                            {
                                for(size_t n3=0; n3<foo.shape[3]; n3++)
                                {
                                    foo(n0,n1,n2,n3) = elem;
                                    elem++;
                                }
                            }
                        }
                    }

                    // Remember that TensorBase<T> inherits from std::vector<T>
                    elem=0;
                    for(auto it=foo.begin(); it!=foo.end(); it++)
                    {
                        *it = elem;
                        elem++;
                    }

                    std::vector<size_t> index = {1,2,4,6};
                    foo(index) = 5.0;

                    std::vector<size_t*> index_ptr = { &index[0],&index[1],&index[2],&index[3] };
                    foo(index_ptr) = 5.0;

                    long double* ptr_to_subtensor = &foo({1,2});

                    return 0;
                }
                \endcode
            */
            T& operator()(const std::vector<size_t> &indices);

            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()(const std::vector<size_t*> &indices);

            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()();
            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()(size_t n0);
            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()(size_t n0, size_t n1);
            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()(size_t n0, size_t n1, size_t n2);
            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()(size_t n0, size_t n1, size_t n2, size_t n3);
            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()(size_t n0, size_t n1, size_t n2, size_t n3, size_t n4);
            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()(size_t n0, size_t n1, size_t n2, size_t n3, size_t n4, size_t n5);
            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()(size_t n0, size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6);
            //! See \ref operator()(const std::vector<size_t> &).
            T& operator()(  size_t n0, size_t n1, size_t n2, size_t n3,
                            size_t n4, size_t n5, size_t n6, size_t n7);

            /*!
                Storage for all components in lexicographical order.
                The initialized memory will be exactly the same as for multidimensional arrays. See also \ref operator<< and \ref operator>>.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> my_tensor({10,10});

                    my_tensor.arange(1); // same as the following

                    my_tensor[0]=1;
                    my_tensor[1]=2;
                    // ...
                    my_tensor[99]=100;

                    return 0;
                }
                \endcode
            */

            /*!
                Specifies the range for all indices.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> foo({2,3});

                    bool i_am_true = foo.shape == std::vector<size_t>{2,3} ;

                    for(unsigned i=0; i<foo.shape[0]; i++)
                    {
                        for(unsigned j=0; j<foo.shape[1]; j++)
                        {
                            foo(i,j);
                        }
                    }

                    return 0;
                }
                \endcode
            */
            std::vector<size_t> shape;

            /*!
                Internally accelerates access of components.
                \code
                #include "TensorUtils.hpp"

                int main()
                {
                    TensorUtils::tensor<double> foo({10,10,10});
                    foo.arange();

                    bool i_am_true = foo(2,3,5) == foo[2*foo.incr[0]+3*foo.incr[1]+5*foo.incr[2]];

                    return 0;
                }
                \endcode
            */
            std::vector<size_t> incr;

        protected:
            //! \private
            void read_txt_helper(std::string path);
            //! \private
            void write_txt(std::string oname, std::string folder);
            //! \private
            template<class BUFFER_TYPE> void print_helper();
            //! \private
            template<class BUFFER_TYPE> void read_txt(std::string path);
            //! \private
            template<class BUFFER_TYPE> void read_bin(std::string path);
            //! \private
            template<class BUFFER_TYPE> void write_bin(std::string basename, std::string folder);
            //! \private
            template<class BUFFER_TYPE> void write_txt(std::string oname, std::string folder, int precision);
    };
}


#endif // TENSORBASE_HPP
