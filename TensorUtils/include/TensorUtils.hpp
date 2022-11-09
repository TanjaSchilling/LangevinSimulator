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

#ifndef TENSORUTILS_HPP
#define TENSORUTILS_HPP

#include "ErrorHandler.hpp"
#include "TensorDerived.hpp"

/*!
    \addtogroup TensorUtils
    @{ \brief This is the main namespace that wraps the entire implementation of this project.
*/
namespace TensorUtils
{
    /*!
        \addtogroup TensorUtils
        @{
    */

    /*!
        \brief Alias declaration for derived class "TensorDerived<T,N>",
        where "T" is the type of the components and "N" is the rank.
        "TensorDerived<T,N>" inherits all its functionality from the base
        class "TensorBase<T>".

        Construct tensors with arbitrary or fixed rank:
        \code
        #include "TensorUtils.hpp"

        int main()
        {
            TensorUtils::tensor<double> my_tensor;

            return 0;
        }
        \endcode
    */
    template<class T, int N=-1> using tensor = TensorDerived<T,N>;
    /*! @} */
}
/*! @} */

#endif // TENSORUTILS_HPP

/*! \mainpage TensorUtils Version 0.1
    \date 22.02.2022
    \author    Christoph Widder
    \copyright GNU Public License.


    TensorUtils is free software. See \ref license for the terms of use.
    You are welcome to report any bugs to <tensorutils@gmail.com>.
    Please visit <https://github.com/TensorUtils/TensorUtils> for the latest
    version.

    \tableofcontents

    \section intro Introduction

	TensorUtils presents a tensor class which is derived from std::vector<T>.
	It allows the usage of all std::vector routines, but has its own constructors.
	The tensor class allows to allocate, initialize, read and write tensors of
	floating or integral types up to rank 8. It provides text and binary file
	formats as well as element-wise operations with support for type conversions
	and chaining. The usage of this library might help to avoid memory leaks,
	segmentation faults, nested loops as well as error-prone index conversions.
	All methods are explicitly instantiated and stored in a shared library, which
	minimizes the compile time of your source code. Additionally, you will find
	routines to transpose, reshape and slice tensors as well as a generalized
	tensor product.

    Supported floating point types are:

    | Data Type         | Extension |
    | :----             | :----     |
    | float             | .f32      |
    | double            | .f64      |
    | long double       | .f80      |

    Supported integral types are:

    | Data Type         | Extension | Data Type             | Extension |
    | :----             | :----     | :----                 | :----     |
    | signed char       | .sc       | unsigned char         | .uc       |
    | short             | .s        | unsigned short        | .us       |
    | int               | .int      | unsigned              | .u        |
    | long              | .l        | unsigned long         | .ul       |
    | long long         | .ll       | unsigned long long    | .ull      |

    The whole project is wrapped into the namespace \ref TensorUtils from "TensorUtils.hpp".
    See the main class \ref TensorUtils::TensorBase<T> for routines and examples.
    Although this base class is fully functional, it is recommended to use
    the derived class \ref TensorUtils::TensorDerived<T,N> which allows you to use
    tensors of arbitrary rank as well as tensors with fixed rank.
    This will be helpful if you need distinct types for tensors of different ranks.
    More details on error-handling can be found in \ref ErrorHandler.

    Once the library is installed, you can use the following alias declarations for
    the class \ref TensorUtils::TensorDerived<T,N>.

        #include "TensorUtils.hpp"
        int main()
        {
            using namespace TensorUtils;
            tensor<double> foo;   // arbitrary rank:  TensorDerived<double,-1>
            tensor<double,4> bar; // fixed rank:      TensorDerived<double,4>
            return 0;
        }

    \section compile Compile

    From within the project folder, type:

        make

    This will create a shared library at:

        PATH_TO_TENSOR_UTILS/lib/Release/libtensor_utils.so
        PATH_TO_TENSOR_UTILS/lib/Debug/libtensor_utilsd.so

    \section install Installation (UBUNTU)

    If you don't want to install the library
    or if you don't want to use the default location, see \ref no_install.

    To install the library at the default locations "/usr/local/lib" and "/usr/local/include", type:

        sudo make install
        make clean

    The header files are now installed as read only (444) in:

        /usr/local/lib/tensor_utils

    The shaed library is installed with read and execute permissions (555) at:

        /usr/local/lib/libtensor_utils.so	# use this library for your release
        /usr/local/lib/libtensor_utilsd.so	# use this library for debugging

    To deinstall the library type:

        sudo make uninstall

    Include the header files:

        -I/usr/local/include/tensor_utils

    Link the shared library:

        -L/usr/local/lib/
        -ltensor_utils
        -ltensor_utilsd

    Your compile commands could look something like:

        # debug
        g++ -Wall -std=c++17 -fexceptions -g -I/usr/local/include/tensor_utils -c main.cpp -o obj/Debug/main.o
        g++ -L/usr/local/lib -o bin/Debug/main obj/Debug/main.o   -ltensor_utilsd

        # release
        g++ -Wall -std=c++17 -fexceptions -O3 -I/usr/local/include/tensor_utils -c main.cpp -o obj/Release/main.o
        g++ -L/usr/local/lib -o bin/Release/main obj/Release/main.o   -ltensor_utils

    You are ready to run your executable!


    \section no_install Usage without installation / Installation with user-defined paths

    Include the header files:

        -I/PATH_TO_TENSOR_UTILS/include

    Link the shared library:

        -L/PATH_TO_TENSOR_UTILS/lib/Release
        -L/PATH_TO_TENSOR_UTILS/lib/Debug
        -ltensor_utils
        -ltensor_utilsd

    Your compile commands could look something like:

        # debug
        g++ -Wall -std=c++17 -fexceptions -g -I/PATH_TO_TENSOR_UTILS/include -c main.cpp -o obj/Debug/main.o
        g++ -L/PATH_TO_TENSOR_UTILS/lib/Debug -o bin/Debug/main obj/Debug/main.o   -ltensor_utilsd

        # release
        g++ -Wall -std=c++17 -fexceptions -O3 -I/usr/local/include/tensor_utils -c main.cpp -o obj/Release/main.o
        g++ -L/PATH_TO_TENSOR_UTILS/lib/Release -o bin/Release/main obj/Release/main.o   -ltensor_utils

    To run your executable, you need to make sure that your operating system will find the shared library.

    On UBUNTU:

        # Release
        cd PATH_TO_TENSOR_UTILS/lib/Release
        export LD_LIBRARY_PATH="$(pwd)"

        # Debug
        cd PATH_TO_TENSOR_UTILS/lib/Debug
        export LD_LIBRARY_PATH="$(pwd)"

    You are ready to run your executable!

    In order to install the library path permanently, create a .conf file in

        /etc/ld.so.conf.d/your_config.conf

    add the following paths in this file

        PATH_TO_TENSOR_UTILS/lib/Release
        PATH_TO_TENSOR_UTILS/lib/Debug

    and update the cache:

        sudo ldconfig

    \section examples Examples

    \code
        #include "TensorUtils.hpp"
        #include <iostream>

        using namespace std;
        using namespace TensorUtils;
        using namespace ErrorHandler;

        void write_test_data()
        {
            tensor<long double> A;
            A.alloc({2,3,5,7});
            A.arange();
            try{
                A.write("A.txt", ".");
                A.write("A.f32", ".");
                A.write("A.f64", ".");
                A.write("A.f80", ".");
                A.write("A.uc", ".");
                A.write("A.sc", ".");
                A.write("A.us", ".");
                A.write("A.s", ".");
                A.write("A.u", ".");
                A.write("A.int", ".");
                A.write("A.ul", ".");
                A.write("A.l", ".");
                A.write("A.ull", ".");
                A.write("A.ll", ".");
            } catch(exception &ex){cout<<ex.what()<<endl;}
        }

        int main()
        {
            write_test_data();

            // CONSTRUCT, ALLOCATE AND INITIALIZE

            tensor<long double> A;
            tensor<double> B({2,3,5,7});
            tensor<float> C(B.shape, 1.0f);

            A.alloc(B.shape);
            A.alloc(B.shape, 2.0L);

            B.init(3.0);

            A=B=C;      // OK! Short for: B=C; A=B;

            A = vector<long double>(A.size(), 1.0L); // initialize from a vector

            if( A == vector<long double>(A.size(), 1.0L)) // bit-wise comparison
            {
                //
            }

            A.arange(); // initialize with 0,1,2,3,... in lexicographical order.

            long double raw_data[A.size()];
            A >> raw_data[0]; // copy data to array
            A << raw_data[0]; // initialize from array

            long double multi_array[2][3][5][7];
            A >> multi_array[0][0][0][0]; // copy data to multi-dimensional array
            A << multi_array[0][0][0][0]; // initialize from multi-dimensional array

            A.print();
            if(!A.empty())
            {
                A.clear();
            }

            //  READ AND WRITE

            A.read("A.txt");    // text file
            A.read("./A.f32");  // binary: float
            A.read("./A.f64");  // binary: double
            A.read("./A.f80");  // binary: long double

            A.write("A.txt", "./");     // text file. If floating point type: write std::numeric_limits<T> significant digits
            A.write("A.txt", "./", 10); // text file. If floating point type: write 10 significant digits
            A.write("A.f32", ".");      // binary: float
            A.write("A.f64", ".");      // binary: double
            A.write("A.f80", ".");      // binary: long double

            //  OPERATORS

            B += B;
            B -= B;
            B = B+B;
            B = B-B;
            B *= 2.0;
            B /= 2.0;     // use *= instead for best performance!
            B = 2.0*B;
            B = B*2.0;
            B = B/2.0;    // use * instead for best performance!

            if(A.shape == B.shape && B.shape == C.shape)
            {
                // Operators will use implicit type conversion of components if necessary:
                A += B;
                A -= B;
                A = B-C;
                A = B+C;
                A = 2*A + 2*( (1.0/3)*B - C );
                C = (-2.0/3)*( 3*C - B ) + 2*A; // same but faster (operators return tensors of the smaller type)
            }
            else
            {
                throw ShapeMismatch("Shape mismatch!");
            }

            // ACCESS ELEMENTS

            int elem = 0;
            for(size_t n0=0; n0<A.shape[0]; n0++)
            {
                for(size_t n1=0; n1<A.shape[1]; n1++)
                {
                    for(size_t n2=0; n2<A.shape[2]; n2++)
                    {
                        for(size_t n3=0; n3<A.shape[3]; n3++)
                        {
                            A(n0,n1,n2,n3) = elem;
                            elem++;
                        }
                    }
                }
            }

            elem=0;
            for(auto it=A.begin(); it!=A.end(); it++)
            {
                *it = elem;
                elem++;
            }

            // SUBTENSORS

            tensor<int> G({6,2,3,5,7});
            A.alloc({2,3,5,7}, 1.0);
            G.arange();

            A.assign(G,{0},{1,1});
            A.add(G, {}, {4});
            A.substract(G, {}, {4});
            A.multiply(2.0, {});
            A.divide(0.5, {});
            A = A.plus(G,{},{1});
            A = A.minus(G,{},{1});
            A = A.product(2.0,{0,0});
            A = G.slice({1,1});
            A = A.quotient(2.0,{1,2});

            //  TRANSPOSE AND RESHAPE

            tensor<float> H({2,3,5,7},0);
            H.arange();
            H = H.transpose({3,1,2,0});
            H.reshape({7*3,5*2});

            //  GENERALIZED TENSOR PRODUCT

            tensor<double> X({2,3,5,7},1);
            tensor<double> Y({2,3,5,7},2);
            tensor<double> Z;

            Z = X.dot(Y,{-1,-2,-3,-4},{-1,-2,-3,-4});   // full contraction: Z is a scalar!

            Z = X.dot(Y,{1,2,3,4},{1,2,3,4});       // Hadamard product: Z has shape {2,3,5,7}

            Z = X.dot(Y,{1,2,3,4},{8,7,6,5});       // tensor product: Z has shape {2,3,5,7,7,5,3,2}

            Z = X.dot(Y,{1,2,3,4},{5,6,7,8},{1,2,4,6}); // compute sub-tensor of tensor-product

            X.alloc({2,3,7,7},1);
            Y.alloc({7,5,3,11},2);

            Z = X.dot(Y,{3,2,-5,-5},{-5,4,2,1});   // generalized tensor product: Z has shape {11,3,2,5}

            //  TENSORS WITH FIXED RANK AND DISTINGUISHABLE TYPES:
            //      In many situations you might want to keep the types of tensors with different rank distinguishable,
            //      i.e. to overload functions that depend on the rank of its arguments.
            //      Everything works exactly the same, but tensors have fixed ranks!

            tensor<double> E({2,3,5},1);
            tensor<float,4> F({2,3,5,7},0); // tensor with fixed rank 4
            try
            {
                F = E;              // throws
                F.alloc({2,3,5});   // throws
                F.alloc({2,3,5},0);  // throws
                tensor<long double,4> G({2,3,5});   // throws
                tensor<long double,4> H({2,3,5},0); // throws
            }
            catch(RankMismatch &ex)
            {
                //
            }
            E = F; // OK!

            //  ERROR HANDLING (see TensorUtils::ErrorHandling for more)
            //      Most error handling is enabled only for the debug-library libtensor_utilsd.so
            //      This will enable you to trace down any occurrence of invalid indices or shape mismatches.

            try
            {
                A(1,2,3,5);
            }
            catch(ShapeMismatch &ex)// wrong number of indices
            {
                cout << ex.what() << endl;
            }
            catch(out_of_range &ex) // at least one index is out of range
            {
                cout << ex.what() << endl;
            }

            try
            {
                A.read("./A.txt");
                A.read("./A.f32");
            }
            catch(UnableToOpenFile & ex) // probably the required file does not exist
            {
                throw ex;
            }
            catch(ShapeMismatch & ex)   // shape does not match data: corrupted file?
            {
                throw ex;
            }
            catch(exception & ex) // catch any other exception
            {
                throw ex;
            }

            return 0;
        }
    \endcode

    \section error_handling Error Handling

    \code
        #include "TensorUtils.hpp"
        #include <iostream>

        using namespace std;
        using namespace TensorUtils;
        using namespace ErrorHandler;

        int main()
        {
            tensor<double> A;

            // READING FILES
            try
            {
                A.read("my_tensor.txt");
            }
            catch(UnableToOpenFile &ex) // unable to open file
            {
                cerr << ex.what() << endl;
            }
            catch(ShapeMismatch & ex)   // shape does not match data: corrupted file?
            {
                throw ex;
            }
            catch(exception & ex) // catch any other exception
            {
                throw ex;
            }

            // ACCESSING COMPONENTS
            A.alloc({2,3,5,7},1.0);
            try
            {
                A(1,2);         // OK! Returns A(1,2,0,0) by reference!
                A(0,0,0,0,0);   // too many indices: throws ShapeMismatch
                A(1,2,4,7);     // index out of range: throws std::out_of_range
            }
            catch(ShapeMismatch &ex) // more indices than expected!
            {
                cerr << ex.what() << endl;
            }
            catch(out_of_range &ex) // at least one index is out of range
            {
                cerr << ex.what() << endl;
            }

            // OPERATORS AND MEMBER FUNCTIONS
            tensor<double>          B({2,3,5,8},1.0);
            tensor<float>           C({2*3,5*7},1.0);
            tensor<long double>     D({},1.0); // scalar
            tensor<int,3>           E({3,5,7},1.0);
            tensor<unsigned long>   F({3,5,7},1.0);
            try
            {
                A += B; // different number of components: throws ShapeMismatch.
                A += C; // OK! Same number of elements, but different shapes!
                E = A;  // RankMismatch: unable to assign with a tensor of different rank!
                E = F;  // OK! Different types, but the ranks are the same.
                A = E;  // OK! A can have arbitrary rank.

                E.alloc({2,3,5,7}); // RankMismatch: E has a fixed rank!

                A.alloc({2,3,5,7},1.0);
                A.assign(B, {1,2}, {1,2});  // ShapeMismatch: assignment with sub-tensor of invalid shape.
                A.assign(C, {1,2}, {0});    // OK! Same number of elements.
                A.assign(C, {1,3}, {0});    // invalid index: throws std::out_of_range.

                F = F.transpose({0,2,1}); // OK! Swap last two axes.
                F = F.transpose({1,3,2}); // ShapeMismatch: Reshape must be a permutation of (0,1,...,N-1).

                C = A.dot(A, {1,2,3}, {1,2,3,4});   // ShapeMismatch: axes must have the same size as the shapes.

                C = A.dot(A, {1,2,3,4}, {5,6,7,8}, {0,0,0,7});  // invalid index: std::out_of_range.
            }
            catch(ShapeMismatch &ex)
            {
                cerr << ex.what() << endl;
            }
            catch(RankMismatch &ex)
            {
                cerr << ex.what() << endl;
            }
            catch(out_of_range &ex)
            {
                cerr << ex.what() << endl;
            }

            return 0;
        }
    \endcode

    \section license License

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
