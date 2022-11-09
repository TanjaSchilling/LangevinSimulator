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

#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include <stdexcept>
#include <string>

namespace TensorUtils
{
    /*!
        \addtogroup TensorUtils
        @{
    */
    /*!
        \addtogroup ErrorHandler
        @{
        \brief This namespace contains error handler classes that inherit from "std::runtime_error".
        Most error handling is enabled only for the debug library "libtensor_utilsd.so".

        TensorUtils provides error handling to trace down rank or shape mismatches, invalid indices
        and invalid file paths.
    */
    namespace ErrorHandler
    {
        /*!
            \addtogroup ErrorHandler
            @{
        */

        //! This error is thrown, if a file cannot be opened. Inherits from std::runtime_error.
        /*!
            See \ref ErrorHandler for details.
        */
        class UnableToOpenFile : public std::runtime_error
        {
            public:
            //! Constructor inherited from std::runtime_error.
            explicit UnableToOpenFile (const std::string& what_arg) : std::runtime_error(what_arg) {};
        };

        //! This error is thrown if any tensor operation is called with invalid shapes or an invalid number of indices. Inherits from std::runtime_error.
        /*! If an index is out of range, std::out_of_range is thrown instead.
            Invalid usage of tensors with fixed ranks have their own error class \ref RankMismatch.
            See \ref ErrorHandler for details.
        */
        class ShapeMismatch : public std::runtime_error
        {
            public:
            //! Constructor inherited from std::runtime_error.
            explicit ShapeMismatch (const std::string& what_arg) : std::runtime_error(what_arg) {};
        };

        //! This error is thrown if any method would change the rank of a tensor with fixed rank. Inherits from std::runtime_error.
        /*!
            See \ref ErrorHandler for details.
        */
        class RankMismatch : public std::runtime_error
        {
            public:
            //! Constructor inherited from std::runtime_error.
            explicit RankMismatch (const std::string& what_arg) : std::runtime_error(what_arg) {};
        };
        /*! @} */
    }
    /*! @} */
    /*! @} */
}

#endif // ERRORHANDLER_HPP
