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

#ifndef THROW_EXCEPTIONS
#define THROW_EXCEPTIONS 0
#endif // THROW_EXCEPTIONS

#ifndef THROW_BASIC_EXCEPTIONS
#define THROW_BASIC_EXCEPTIONS 1
#endif // THROW_BASIC_EXCEPTIONS

#include "TensorBase.hpp"
#include "ErrorHandler.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <set>
#include <functional>
#include <cstring>
#include <iomanip>

using namespace std;
using namespace TensorUtils;
using namespace ErrorHandler;

/**
    CONSTRUCTOR & DESTRUCTOR
**/

template<class T>
TensorBase<T>::TensorBase() : vector<T>()
{
    //
}

template<class T>
TensorBase<T>::TensorBase(const vector<size_t> &shape) : vector<T>()
{
    alloc(shape);
}

template<class T>
TensorBase<T>::TensorBase(const vector<size_t> &shape, const T& val) : vector<T>()
{
    alloc(shape, val);
}

template<class T>
TensorBase<T>::~TensorBase()
{
    //dtor
}

/**
    ALLOCATE AND INITIALIZE
**/

template<class T>
void TensorBase<T>::alloc(const vector<size_t> &shape)
{
    if(shape.empty())
    {
        vector<T>::resize(1); // scalar
        this->shape.clear();
        incr.clear();
        return;
    }
    this->shape=shape;
    unsigned dim = shape.size();
    incr.resize(dim);
    incr.back()=1;
    dim--;
    size_t num_elems = 1;
    while(dim > 0)
    {
        dim--;
        incr[dim] = shape[dim+1]*incr[dim+1];
        num_elems *= shape[dim+1];
    }
    num_elems *= shape.front();
    vector<T>::resize(num_elems);
}

template<class T>
void TensorBase<T>::alloc(const vector<size_t> &shape, const T& val)
{
    alloc(shape);
    init(val);
}

template<class T>
void TensorBase<T>::init(const T& val)
{
    vector<T>::assign(vector<T>::size(), val);
}

template<class T>
void TensorBase<T>::arange(T val)
{
    for(auto it=vector<T>::begin();it!=vector<T>::end();it++)
    {
        *it=val;
        val++;
    }
}

template<class T>
void TensorBase<T>::clear()
{
    vector<T>::clear();
    shape.clear();
    incr.clear();
}

/**
    PRINT
**/

template<class T>               // class template
template<class BUFFER_TYPE>     // function template inside the class template
void TensorBase<T>::print_helper()
{
    for(auto it=shape.begin();it!=shape.end();it++)
    {
        cout << *it << '\t';
    }
    cout << '\n' << '\n';
    for(size_t n=0;n<vector<T>::size();n++)
    {
        cout << (BUFFER_TYPE)(*this)[n] << '\t';
        if(incr.empty())
        {
            break;
        }
        for(auto it=incr.begin(); it!= --incr.end(); it++)
        {
            if((n+1)%(*it) == 0)
            {
                cout << '\n';
            }
        }
    }
}

template<class T>               // get rid of function template argument
void TensorBase<T>::print()
{
    TensorBase<T>::print_helper<T>();
}
template<>                      // specialize class template
void TensorBase<unsigned char>::print()
{
    TensorBase<unsigned char>::print_helper<int>();
}
template<>                      // specialize class template
void TensorBase<signed char>::print()
{
    TensorBase<signed char>::print_helper<int>();
}

/**
    READ
**/

// MAIN FUNCTION TO BE CALLED
template<class T>
void TensorBase<T>::read(string path)
{
    string extension = filesystem::path(path).extension();
    if(extension == ".f32")    {read_bin<float>(path);}
    else if(extension == ".f64")    {read_bin<double>(path);}
    else if(extension == ".f80")    {read_bin<long double>(path);}
    else if(extension == ".uc")     {read_bin<unsigned char>(path);}
    else if(extension == ".sc")     {read_bin<signed char>(path);}
    else if(extension == ".us")     {read_bin<unsigned short>(path);}
    else if(extension == ".s")      {read_bin<short>(path);}
    else if(extension == ".u")      {read_bin<unsigned>(path);}
    else if(extension == ".int")    {read_bin<int>(path);}
    else if(extension == ".ul")     {read_bin<unsigned long>(path);}
    else if(extension == ".l")      {read_bin<long>(path);}
    else if(extension == ".ull")    {read_bin<unsigned long long>(path);}
    else if(extension == ".ll")     {read_bin<long long>(path);}
    else
    {
        read_txt_helper(path);
    }
}

// READ TEXT TO A BUFFER AND STORE DATA USING IMPLICIT TYPE CONVERSIONS
template<class T>
template<class BUFFER_TYPE>
void TensorBase<T>::read_txt(string path)
{
    ifstream in(path);
    if(in.fail())
    {
        string err_str = "TensorUtils::TensorBase<T>::read_txt:: Unable to open file \"";
        err_str.append(path);
        err_str.append("\".");
        throw UnableToOpenFile(err_str);
    }
    string line;
    stringstream ss;

    getline(in,line);
    ss = stringstream(line);
    shape.clear();
    size_t dummy;
    while(ss >> dummy)
    {
        shape.push_back(dummy);
    }
    alloc(shape);

    size_t idx = 0;
    BUFFER_TYPE tmp;
    bool idx_out_of_range = false;
    while(getline(in, line))
    {
        ss = stringstream(line);
        if(line == "")
        {
            continue;
        }
        else
        {
            while (ss >> tmp)
            {
                if(idx < vector<T>::size())
                {
                    (*this)[idx] = tmp;
                    idx++;
                }
                else
                {
                    idx_out_of_range = true;
                    break;
                }
            }
        }
        if(idx_out_of_range)
        {
            break;
        }
    }
    if(idx_out_of_range)
    {
        string err_str = "TensorUtils::TensorBase<T>::read_txt:: More data than expected from shape in file \"";
        err_str.append(path);
        err_str.append("\".");
        throw ShapeMismatch(err_str);
    }
    if(idx<vector<T>::size())
    {
        string err_str = "TensorUtils::TensorBase<T>::read_txt:: Less data than expected from shape in file \"";
        err_str.append(path);
        err_str.append("\".");
        throw ShapeMismatch(err_str);
    }
}

// READ WITH BUFFER_TYPER EQUAL TO CLASS TEMPLATE TYPE T, EXCEPT FOR CLASS TEMPLATE SPECIALIZATIONS
template<class T>
void TensorBase<T>::read_txt_helper(string path)
{
    TensorBase<T>::read_txt<T>(path);
}
template<>
void TensorBase<unsigned char>::read_txt_helper(string path)
{
    TensorBase<unsigned char>::read_txt<long double>(path);
}
template<>
void TensorBase<signed char>::read_txt_helper(string path)
{
    TensorBase<signed char>::read_txt<long double>(path);
}

// READ BINARY DATA TO A BUFFER OF TYPE BUFFER_TYPE AND STORE DATA USING IMPLICIT TYPE CONVERSIONS
template<class T>
template<class BUFFER_TYPE>
void TensorBase<T>::read_bin(string path)
{
    ifstream in(path, ios::in | ios::binary);
    if(in.fail())
    {
        string err_str = "TensorUtils::TensorBase<T>::read_binary:: Unable to open file \"";
        err_str.append(path);
        err_str.append("\".");
        throw UnableToOpenFile(err_str);
    }

    size_t header_size;
    size_t data_size;

    // read header
    in.read((char*)&header_size, sizeof(size_t));
    shape.resize(header_size);
    in.read((char*)&shape[0], header_size*sizeof(size_t));
    in.read((char*)&data_size, sizeof(size_t));

    alloc(shape);

    // read data
    bool data_too_large = false;
    bool data_too_small = false;
    if(vector<T>::size() < data_size)
    {
        data_too_large = true;
    }
    else if(vector<T>::size() > data_size)
    {
        data_too_small = true;
    }
    size_t num_expected = vector<T>::size();

    vector<T>::resize(data_size);

    vector<BUFFER_TYPE> buffer(data_size);
    in.read((char*)&buffer[0], data_size*sizeof(BUFFER_TYPE));
    auto it2 = vector<T>::begin();
    for(auto it=buffer.begin(); it!= buffer.end(); it++)
    {
        *it2 = *it;
        it2++;
    }

    in.close();

    if(data_too_large)
    {
        vector<T>::resize(num_expected);
        string err_str = "TensorUtils::TensorBase<T>::read_binary:: More data than expected from shape in file \"";
        err_str.append(path);
        err_str.append("\".");
        throw ShapeMismatch(err_str);
    }
    if(data_too_small)
    {
        while(vector<T>::size()<num_expected)
        {
            this->push_back(0);
        }
        string err_str = "TensorUtils::TensorBase<T>::read_binary:: Less data than expected from shape in file \"";
        err_str.append(path);
        err_str.append("\".");
        throw ShapeMismatch(err_str);
    }
}

/**
    WRITE
**/

// MAIN FUNCTION TO WRITE ANY DATA
template<class T>
void TensorBase<T>::write(string oname, string folder)
{
    string extension = filesystem::path(oname).extension();
    if(extension == ".f32")    {write_bin<float>(oname, folder);}
    else if(extension == ".f64")    {write_bin<double>(oname, folder);}
    else if(extension == ".f80")    {write_bin<long double>(oname, folder);}
    else if(extension == ".uc")     {write_bin<unsigned char>(oname, folder);}
    else if(extension == ".sc")     {write_bin<signed char>(oname, folder);}
    else if(extension == ".us")     {write_bin<unsigned short>(oname, folder);}
    else if(extension == ".s")      {write_bin<short>(oname, folder);}
    else if(extension == ".u")      {write_bin<unsigned>(oname, folder);}
    else if(extension == ".int")    {write_bin<int>(oname, folder);}
    else if(extension == ".ul")     {write_bin<unsigned long>(oname, folder);}
    else if(extension == ".l")      {write_bin<long>(oname, folder);}
    else if(extension == ".ull")    {write_bin<unsigned long long>(oname, folder);}
    else if(extension == ".ll")     {write_bin<long long>(oname, folder);}
    else
    {
        write_txt(oname, folder);
    }
}

template<class T>
void TensorBase<T>::write_txt(string oname, string folder)
{
    TensorBase<T>::write_txt<T>(oname, folder, 16);
}
template<>
void TensorBase<float>::write_txt(string oname, string folder)
{
    TensorBase<float>::write_txt<float>(oname, folder, numeric_limits<float>::max_digits10);
}
template<>
void TensorBase<double>::write_txt(string oname, string folder)
{
    TensorBase<double>::write_txt<double>(oname, folder, numeric_limits<double>::max_digits10);
}
template<>
void TensorBase<long double>::write_txt(string oname, string folder)
{
    TensorBase<long double>::write_txt<long double>(oname, folder, numeric_limits<long double>::max_digits10);
}
template<>
void TensorBase<unsigned char>::write_txt(string oname, string folder)
{
    TensorBase<unsigned char>::write_txt<int>(oname, folder, 16);
}
template<>
void TensorBase<signed char>::write_txt(string oname, string folder)
{
    TensorBase<signed char>::write_txt<int>(oname, folder, 16);
}

// MAIN FUNCTION TO WRITE TEXT FILES WITH SPECIFIED NUMBER OF DIGITS
template<class T>
void TensorBase<T>::write(
    string oname,
    string folder,
    int precision)
{
    TensorBase<T>::write_txt<T>(oname, folder, precision);
}
template<>
void TensorBase<unsigned char>::write(
    string oname,
    string folder,
    int precision)
{
    TensorBase<unsigned char>::write_txt<int>(oname, folder, precision);
}
template<>
void TensorBase<signed char>::write(
    string oname,
    string folder,
    int precision)
{
    TensorBase<signed char>::write_txt<int>(oname, folder, precision);
}

// CONVERT DATA TO BUFFER_TYPE, THEN WRITE TO TEXT FILE WITH SPECIFIED PRECISION
template<class T>
template<class BUFFER_TYPE>
void TensorBase<T>::write_txt(
    string oname,
    string folder,
    int precision)
{
    string extension = filesystem::path(oname).extension();
    if(extension==".f32"||
       extension==".f64"||
       extension==".f80"||
       extension==".uc"||
       extension==".sc"||
       extension==".us"||
       extension==".s"||
       extension==".u"||
       extension==".int"||
       extension==".ul"||
       extension==".l"||
       extension==".ull"||
       extension==".ll")
    {
        throw std::runtime_error("Invalid file extension: extension for binary file format, but text file requested!");
    }

    filesystem::create_directories(folder);
    string path = folder;
    if(path.back() != '/' ){
        path.append("/");
    }
    path.append(oname);
    ofstream out;
    out << scientific;
    out << setprecision(precision);
    out.open(path);

    for(auto it=shape.begin();it!=shape.end();it++)
    {
        out << *it << '\t';
    }
    out << '\n' << '\n';
    for(size_t n=0;n<vector<T>::size();n++)
    {
        out << (BUFFER_TYPE)(*this)[n] << '\t';
        if(incr.empty())
        {
            break;
        }
        for(auto it=incr.begin(); it!= --incr.end(); it++)
        {
            if((n+1)%(*it) == 0)
            {
                out << '\n';
            }
        }
    }
    out.close();
}

// CONVERT DATA TO BUFFER_TYPE, THEN WRITE TO BINARY
template<class T>
template<class BUFFER_TYPE>
void TensorBase<T>::write_bin(string oname, string folder)
{
    filesystem::create_directories(folder);
    string path = folder;
    if(path.back() != '/' ){
        path.append("/");
    }
    path.append(oname);

    ofstream out(path, ios::out | ios::binary);

    size_t header_size = shape.size();
    size_t data_size = vector<T>::size();

    // write header
    out.write((char*)&header_size, sizeof(size_t));
    out.write((char*)&shape[0], header_size*sizeof(size_t));
    out.write((char*)&data_size, sizeof(size_t));

    // write data
    vector<BUFFER_TYPE> buffer(vector<T>::size());
    auto it2 = vector<T>::begin();
    for(auto it=buffer.begin();it!=buffer.end();it++)
    {
        *it = *it2;
        it2++;
    }
    out.write((char*)&buffer[0], data_size*sizeof(BUFFER_TYPE));

    out.close();
}

template<class T>
TensorBase<T> TensorBase<T>::transpose(const vector<unsigned> &axes)
{
    if(THROW_BASIC_EXCEPTIONS)
    {
        set<unsigned> set1;
        for(unsigned n=0; n<shape.size(); n++)
        {
            set1.insert(n);
        }
        set<unsigned> set2;
        for(auto it=axes.begin(); it!=axes.end(); it++)
        {
            set2.insert(*it);
        }
        if( set1 != set2 )
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::transpose:: Axes do not match!");
        }
    }

    vector<size_t> shape2;
    for(auto it=axes.begin(); it!=axes.end(); it++)
    {
        shape2.push_back(shape[*it]);
    }

    TensorBase<T> result(shape2);

    vector<size_t> index(shape.size());
    vector<size_t> index2(shape2.size());
    
    function<void(unsigned)> iterate = [&](unsigned dim2)
    {
        index[axes[dim2]]=0;
        index2[dim2]=0;
        while(index2[dim2]<shape2[dim2])
        {
            if( dim2 == shape2.size()-1 )
            {
                result(index2) = (*this)(index);
            }
            else
            {
                iterate(dim2+1);
            }
            index[axes[dim2]]++;
            index2[dim2]++;
        }
    };
    iterate(0);

    return result;
}

template<class T>
TensorBase<T> TensorBase<T>::slice(const std::vector<size_t> &idx_at)
{
    if(THROW_BASIC_EXCEPTIONS && idx_at.size()>= shape.size())
    {
        throw ShapeMismatch("TensorBase<T>::slice(const std::vector<unsigned>&):: Too many indices!");
    }
    vector<size_t> final_shape(shape);
    final_shape.erase(final_shape.begin(),final_shape.begin()+idx_at.size());
    TensorBase<T> result(final_shape);
    result.assign((*this), {}, idx_at);
    return result;
}

template<class T>
TensorBase<T>& TensorBase<T>::reshape(const std::vector<size_t> &shape)
{
    if(shape.empty())
    {
        if(THROW_BASIC_EXCEPTIONS && 1 != vector<T>::size())
        {
            throw ShapeMismatch("TensorBase<T>::reshape(const std::vector<size_t> &shape):: Shape does not match the number of components!");
        }
        this->shape.clear();
        incr.clear();
    }
    else
    {
        this->shape=shape;
        unsigned dim = shape.size();
        incr.resize(dim);
        incr.back()=1;
        dim--;
        size_t num_elems = 1;
        while(dim > 0)
        {
            dim--;
            incr[dim] = shape[dim+1]*incr[dim+1];
            num_elems *= shape[dim+1];
        }
        num_elems *= shape.front();
        if(THROW_BASIC_EXCEPTIONS && num_elems != vector<T>::size())
        {
            throw ShapeMismatch("TensorBase<T>::reshape(const std::vector<size_t> &shape):: Shape does not match the number of components!");
        }
    }
    return *this;
}

template<class T>
template<class T2>
TensorBase<T> TensorBase<T>::dot(TensorBase<T2>& B, const vector<int> &idx_lhs, const vector<int> &idx_rhs, const vector<size_t> &idx_at)
{
    if(THROW_BASIC_EXCEPTIONS && (shape.size()!=idx_lhs.size() || B.shape.size()!= idx_rhs.size()))
    {
        throw ShapeMismatch("TensorUtils::TensorBase<T>::dot:: Shape mismatch!");
    }

    // concat axes
    vector<int> ax_all = idx_lhs;
    for(auto it=idx_rhs.begin(); it!=idx_rhs.end(); it++)
    {
        ax_all.push_back(*it);
    }

    // get final and contraction indices
    set<int> idx_contr;
    set<int> idx_final;
    for(auto it=ax_all.begin(); it!=ax_all.end(); it++)
    {
        if(*it<0)
        {
            idx_contr.insert(*it);
        }
        else
        {
            idx_final.insert(*it);
        }
    }

    // get idx_contr positions
    vector<vector<unsigned>> contr_pos(idx_contr.size());
    vector<vector<unsigned>> contr_posA(idx_contr.size());
    vector<vector<unsigned>> contr_posB(idx_contr.size());
    auto iter = contr_pos.begin();
    auto iterA = contr_posA.begin();
    auto iterB = contr_posB.begin();
    for(auto it=idx_contr.begin();it!=idx_contr.end(); it++)
    {
        for(unsigned n=0; n<ax_all.size(); n++)
        {
            if(*it == ax_all[n] )
            {
                (*iter).push_back(n);
            }
        }
        for(unsigned n=0; n<idx_lhs.size(); n++)
        {
            if(*it == idx_lhs[n] )
            {
                (*iterA).push_back(n);
            }
        }
        for(unsigned n=0; n<idx_rhs.size(); n++)
        {
            if(*it == idx_rhs[n] )
            {
                (*iterB).push_back(n);
            }
        }
        iter++;
        iterA++;
        iterB++;
    }

    // get idx_final positions
    vector<vector<unsigned>> final_pos(idx_final.size());
    vector<vector<unsigned>> final_posA(idx_final.size());
    vector<vector<unsigned>> final_posB(idx_final.size());
    iter = final_pos.begin();
    iterA = final_posA.begin();
    iterB = final_posB.begin();
    for(auto it=idx_final.begin();it!=idx_final.end(); it++)
    {
        for(unsigned n=0; n<ax_all.size(); n++)
        {
            if(*it == ax_all[n] )
            {
                (*iter).push_back(n);
            }
        }
        for(unsigned n=0; n<idx_lhs.size(); n++)
        {
            if(*it == idx_lhs[n] )
            {
                (*iterA).push_back(n);
            }
        }
        for(unsigned n=0; n<idx_rhs.size(); n++)
        {
            if(*it == idx_rhs[n] )
            {
                (*iterB).push_back(n);
            }
        }
        iter++;
        iterA++;
        iterB++;
    }

    // concat shapes
    vector<size_t> shape_all = shape;
    for(auto it=B.shape.begin();it!=B.shape.end(); it++)
    {
        shape_all.push_back(*it);
    }
    // get final shape
    vector<size_t> shape_final;
    for(auto it=final_pos.begin(); it!=final_pos.end(); it++)
    {
        size_t shape_0 = shape_all[ (*it)[0] ];
        if(THROW_BASIC_EXCEPTIONS)
        {
            for(auto it2 = it->begin(); it2!=it->end(); it2++)
            {
                if(shape_0 != shape_all[ *it2 ])
                {
                    throw ShapeMismatch("TensorUtils::TensorBase<T>::dot:: Shape mismatch!");
                }
            }
        }
        shape_final.push_back(shape_0);
    }
    // get contraction shape
    vector<size_t> shape_contr;
    for(auto it=contr_pos.begin(); it!=contr_pos.end(); it++)
    {
        size_t shape_0 = shape_all[ (*it)[0] ];
        if(THROW_BASIC_EXCEPTIONS)
        {
            for(auto it2 = it->begin(); it2!=it->end(); it2++)
            {
                if(shape_0 != shape_all[ *it2 ])
                {
                    throw ShapeMismatch("TensorUtils::TensorBase<T>::dot:: Shape mismatch!");
                }
            }
        }
        shape_contr.push_back(shape_0);
    }

    if(THROW_BASIC_EXCEPTIONS && idx_at.size()>shape_final.size())
    {
        throw ShapeMismatch("TensorUtils::TensorBase<T>::dot:: Shape mismatch!");
    }

    // result, indices and pointer to be captured by lambda functions
    vector<size_t> index_final(shape_final.size());
    vector<size_t> index_contr(shape_contr.size());
    vector<size_t*> idxAptr(shape.size());
    vector<size_t*> idxBptr(B.shape.size());
    const unsigned dim_max = shape_final.size()-1;

    // set pointer to indices
    iterA = final_posA.begin();
    iterB = final_posB.begin();
    for(auto it=index_final.begin(); it!=index_final.end(); it++ )
    {
        for(auto it2=iterA->begin();it2!=iterA->end();it2++)
        {
            idxAptr[*it2]=&(*it);
        }
        for(auto it2=iterB->begin();it2!=iterB->end();it2++)
        {
            idxBptr[*it2]=&(*it);
        }
        iterA++;
        iterB++;
    }
    iterA = contr_posA.begin();
    iterB = contr_posB.begin();
    for(auto it=index_contr.begin(); it!=index_contr.end(); it++ )
    {
        for(auto it2=iterA->begin();it2!=iterA->end();it2++)
        {
            idxAptr[*it2]=&(*it);
        }
        for(auto it2=iterB->begin();it2!=iterB->end();it2++)
        {
            idxBptr[*it2]=&(*it);
        }
        iterA++;
        iterB++;
    }

    shape_final.erase(shape_final.begin(), shape_final.begin()+idx_at.size());
    TensorBase<T> result(shape_final); // to be capured

    function<void(unsigned, T&)> iterate_contr = [&](unsigned dim, T &buff)
    {
        index_contr[dim] = 0;
        while(index_contr[dim]<shape_contr[dim])
        {
            if( dim == shape_contr.size()-1 )
            {
                buff += (*this)(idxAptr) * B(idxBptr);
            }
            else
            {
                iterate_contr(dim+1,buff);
            }
            index_contr[dim]++;
        }
    };

    if(idx_at.empty())
    {
        function<void(unsigned)> iterate = [&](unsigned dim)
        {
            index_final[dim] = 0;
            while(index_final[dim]<shape_final[dim])
            {
                if( dim == dim_max )
                {
                    result(index_final) = 0.0;
                    iterate_contr(0,result(index_final));
                }
                else
                {
                    iterate(dim+1);
                }
                index_final[dim]++;
            }
        };
        function<void(unsigned)> iterate_no_contr = [&](unsigned dim)
        {
            index_final[dim] = 0;
            while(index_final[dim]<shape_final[dim])
            {
                if( dim == dim_max )
                {
                    result(index_final) = (*this)(idxAptr) * B(idxBptr);
                }
                else
                {
                    iterate_no_contr(dim+1);
                }
                index_final[dim]++;
            }
        };
        // sum over all final and contraction indices
        if(shape_contr.empty())
        {
            if(shape_final.empty()) // return value and arguments are scalars!
            {
                result[0]=(*this)[0]*B[0];
            }
            else // return value is not a scalar and there are no indices to contract!
            {
                iterate_no_contr(0);
            }
        }
        else if(shape_final.empty()) // return value is a scalar!
        {
            result[0] = 0.0;
            iterate_contr(0,result[0]);
        }
        else // return value is not a scalar and there are indices to contract!
        {
            iterate(0);
        }
    }
    else
    {
        vector<size_t*> index_final_ptr(index_final.size());
        // set index_final for idx_at
        auto iter_final = index_final.begin();
        for(auto it=idx_at.begin(); it!=idx_at.end(); it++)
        {
            *iter_final = *it;
            iter_final++;
        }
        // set index_final_ptr
        auto iter_final_ptr = index_final_ptr.begin();
        for(auto it=index_final.begin(); it!=index_final.end(); it++)
        {
            *iter_final_ptr = &(*it);
            iter_final_ptr++;
        }
        // drop indices given by idx_at
        index_final_ptr.erase(index_final_ptr.begin(), index_final_ptr.begin()+idx_at.size());

        function<void(unsigned)> iterate = [&](unsigned dim)
        {
            *index_final_ptr[dim] = 0;
            while(*index_final_ptr[dim]<shape_final[dim])
            {
                if( dim == dim_max )
                {
                    result(index_final_ptr) = 0.0;
                    iterate_contr(0,result(index_final_ptr));
                }
                else
                {
                    iterate(dim+1);
                }
                (*index_final_ptr[dim])++;
            }
        };
        function<void(unsigned)> iterate_no_contr = [&](unsigned dim)
        {
            *index_final_ptr[dim] = 0;
            while(*index_final_ptr[dim]<shape_final[dim])
            {
                if( dim == dim_max )
                {
                    result(index_final_ptr) = (*this)(idxAptr) * B(idxBptr);
                }
                else
                {
                    iterate_no_contr(dim+1);
                }
                (*index_final_ptr[dim])++;
            }
        };
        // sum over all final and contraction indices
        if(shape_contr.empty())
        {
            if(shape_final.empty()) // return value is a scalars!
            {
                result[0]=(*this)(idxAptr)*B(idxBptr);
            }
            else // return value is not a scalar and there are no indices to contract!
            {
                iterate_no_contr(0);
            }
        }
        else if(shape_final.empty()) // return value is a scalar!
        {
            result[0] = 0.0;
            iterate_contr(0,result[0]);
        }
        else // return value is not a scalar and there are indices to contract!
        {
            iterate(0);
        }
    }

    return result;
}

template<class T>
TensorBase<T> TensorBase<T>::contract(const vector<int> &idx_lhs, const vector<size_t> &idx_at)
{
    if(THROW_BASIC_EXCEPTIONS && shape.size()!=idx_lhs.size() )
    {
        throw ShapeMismatch("TensorUtils::TensorBase<T>::contract:: Shape mismatch!");
    }

    // get final and contraction indices
    set<int> idx_contr;
    set<int> idx_final;
    for(auto it=idx_lhs.begin(); it!=idx_lhs.end(); it++)
    {
        if(*it<0)
        {
            idx_contr.insert(*it);
        }
        else
        {
            idx_final.insert(*it);
        }
    }

    // get idx_contr positions
    vector<vector<unsigned>> contr_pos(idx_contr.size());
    auto iter = contr_pos.begin();
    for(auto it=idx_contr.begin();it!=idx_contr.end(); it++)
    {
        for(unsigned n=0; n<idx_lhs.size(); n++)
        {
            if(*it == idx_lhs[n] )
            {
                (*iter).push_back(n);
            }
        }
        iter++;
    }

    // get idx_final positions
    vector<vector<unsigned>> final_pos(idx_final.size());
    iter = final_pos.begin();
    for(auto it=idx_final.begin();it!=idx_final.end(); it++)
    {
        for(unsigned n=0; n<idx_lhs.size(); n++)
        {
            if(*it == idx_lhs[n] )
            {
                (*iter).push_back(n);
            }
        }
        iter++;
    }

    // get final shape
    vector<size_t> shape_final;
    for(auto it=final_pos.begin(); it!=final_pos.end(); it++)
    {
        size_t shape_0 = shape[ (*it)[0] ];
        if(THROW_BASIC_EXCEPTIONS)
        {
            for(auto it2 = it->begin(); it2!=it->end(); it2++)
            {
                if(shape_0 != shape[ *it2 ])
                {
                    throw ShapeMismatch("TensorUtils::TensorBase<T>::contract:: Shape mismatch!");
                }
            }
        }
        shape_final.push_back(shape_0);
    }
    // get contraction shape
    vector<size_t> shape_contr;
    for(auto it=contr_pos.begin(); it!=contr_pos.end(); it++)
    {
        size_t shape_0 = shape[ (*it)[0] ];
        if(THROW_BASIC_EXCEPTIONS)
        {
            for(auto it2 = it->begin(); it2!=it->end(); it2++)
            {
                if(shape_0 != shape[ *it2 ])
                {
                    throw ShapeMismatch("TensorUtils::TensorBase<T>::contract:: Shape mismatch!");
                }
            }
        }
        shape_contr.push_back(shape_0);
    }

    if(THROW_BASIC_EXCEPTIONS && idx_at.size()>shape_final.size())
    {
        throw ShapeMismatch("TensorUtils::TensorBase<T>::contract:: Shape mismatch!");
    }

    // result, indices and pointer to be captured by lambda functions
    vector<size_t> index_final(shape_final.size());
    vector<size_t> index_contr(shape_contr.size());
    vector<size_t*> idx_ptr(shape.size());
    const unsigned dim_max = shape_final.size()-1;

    // set pointer to indices
    iter = final_pos.begin();
    for(auto it=index_final.begin(); it!=index_final.end(); it++ )
    {
        for(auto it2=iter->begin();it2!=iter->end();it2++)
        {
            idx_ptr[*it2]=&(*it);
        }
        iter++;
    }
    iter = contr_pos.begin();
    for(auto it=index_contr.begin(); it!=index_contr.end(); it++ )
    {
        for(auto it2=iter->begin();it2!=iter->end();it2++)
        {
            idx_ptr[*it2]=&(*it);
        }
        iter++;
    }

    shape_final.erase(shape_final.begin(), shape_final.begin()+idx_at.size());
    TensorBase<T> result(shape_final); // to be capured

    function<void(unsigned, T&)> iterate_contr = [&](unsigned dim, T &buff)
    {
        index_contr[dim] = 0;
        while(index_contr[dim]<shape_contr[dim])
        {
            if( dim == shape_contr.size()-1 )
            {
                buff += (*this)(idx_ptr);
            }
            else
            {
                iterate_contr(dim+1,buff);
            }
            index_contr[dim]++;
        }
    };

    if(idx_at.empty())
    {
        function<void(unsigned)> iterate = [&](unsigned dim)
        {
            index_final[dim] = 0;
            while(index_final[dim]<shape_final[dim])
            {
                if( dim == dim_max )
                {
                    result(index_final) = 0.0;
                    iterate_contr(0,result(index_final));
                }
                else
                {
                    iterate(dim+1);
                }
                index_final[dim]++;
            }
        };
        function<void(unsigned)> iterate_no_contr = [&](unsigned dim)
        {
            index_final[dim] = 0;
            while(index_final[dim]<shape_final[dim])
            {
                if( dim == dim_max )
                {
                    result(index_final) = (*this)(idx_ptr);
                }
                else
                {
                    iterate_no_contr(dim+1);
                }
                index_final[dim]++;
            }
        };
        // sum over all final and contraction indices
        if(shape_contr.empty())
        {
            if(shape_final.empty()) // return value and arguments are scalars!
            {
                result[0]=(*this)[0];
            }
            else // return value is not a scalar and there are no indices to contract!
            {
                iterate_no_contr(0);
            }
        }
        else if(shape_final.empty()) // return value is a scalar!
        {
            result[0] = 0.0;
            iterate_contr(0,result[0]);
        }
        else // return value is not a scalar and there are indices to contract!
        {
            iterate(0);
        }
    }
    else
    {
        vector<size_t*> index_final_ptr(index_final.size());
        // set index_final for idx_at
        auto iter_final = index_final.begin();
        for(auto it=idx_at.begin(); it!=idx_at.end(); it++)
        {
            *iter_final = *it;
            iter_final++;
        }
        // set index_final_ptr
        auto iter_final_ptr = index_final_ptr.begin();
        for(auto it=index_final.begin(); it!=index_final.end(); it++)
        {
            *iter_final_ptr = &(*it);
            iter_final_ptr++;
        }
        // drop indices given by idx_at
        index_final_ptr.erase(index_final_ptr.begin(), index_final_ptr.begin()+idx_at.size());

        function<void(unsigned)> iterate = [&](unsigned dim)
        {
            *index_final_ptr[dim] = 0;
            while(*index_final_ptr[dim]<shape_final[dim])
            {
                if( dim == dim_max )
                {
                    result(index_final_ptr) = 0.0;
                    iterate_contr(0,result(index_final_ptr));
                }
                else
                {
                    iterate(dim+1);
                }
                (*index_final_ptr[dim])++;
            }
        };
        function<void(unsigned)> iterate_no_contr = [&](unsigned dim)
        {
            *index_final_ptr[dim] = 0;
            while(*index_final_ptr[dim]<shape_final[dim])
            {
                if( dim == dim_max )
                {
                    result(index_final_ptr) = (*this)(idx_ptr) ;
                }
                else
                {
                    iterate_no_contr(dim+1);
                }
                (*index_final_ptr[dim])++;
            }
        };
        // sum over all final and contraction indices
        if(shape_contr.empty())
        {
            if(shape_final.empty()) // return value is a scalars!
            {
                result[0]=(*this)(idx_ptr);
            }
            else // return value is not a scalar and there are no indices to contract!
            {
                iterate_no_contr(0);
            }
        }
        else if(shape_final.empty()) // return value is a scalar!
        {
            result[0] = 0.0;
            iterate_contr(0,result[0]);
        }
        else // return value is not a scalar and there are indices to contract!
        {
            iterate(0);
        }
    }

    return result;
}

/**
    OPERATORS
**/

template<class T>
TensorBase<T>& TensorBase<T>::operator= (const std::vector<T>& rhs)
{
    if(vector<T>::size() != rhs.size())
    {
        throw ShapeMismatch("TensorBase<T>::operator=(const std::vector<T>&):: Assigment with invalid number of elements!");
    }
    vector<T>::operator=(rhs);
    return *this;
}

template<class T>    // class template
template<class T2>   // function template
TensorBase<T>& TensorBase<T>::operator=(const TensorBase<T2>& rhs)
{
    vector<T>::resize(rhs.size());
    auto it_rhs = rhs.begin();
    for(auto it=vector<T>::begin(); it!=vector<T>::end(); it++)
    {
        *it = *it_rhs;
        it_rhs++;
    }
    shape = rhs.shape;
    incr = rhs.incr;
    return *this;
}

template<class T>    // class template
template<class T2>   // function template
TensorBase<T>& TensorBase<T>::operator+=(const TensorBase<T2>& rhs)
{
    if(THROW_EXCEPTIONS && vector<T>::size()!=rhs.size())
    {
        throw ShapeMismatch("TensorUtils::TensorBase<T>::operator+=:: Shape mismatch: Arguments have not the same number of elements!");
    }
    auto it_rhs = rhs.begin();
    for(auto it=vector<T>::begin(); it!=vector<T>::end(); it++)
    {
        *it += *it_rhs;
        it_rhs++;
    }
    return *this;
}

template<class T>    // class template
template<class T2>   // function template
TensorBase<T> TensorBase<T>::operator+(const TensorBase<T2>& rhs)
{
    TensorBase<T> result(*this);
    result += rhs;
    return result;
}

template<class T>    // class template
template<class T2>   // function template
TensorBase<T>& TensorBase<T>::operator-=(const TensorBase<T2>& rhs)
{
    if(THROW_EXCEPTIONS && vector<T>::size()!=rhs.size())
    {
        throw ShapeMismatch("TensorUtils::TensorBase<T>::operator-=:: Shape mismatch: Arguments have not the same number of elements!");
    }
    auto it_rhs = rhs.begin();
    for(auto it=vector<T>::begin(); it!=vector<T>::end(); it++)
    {
        *it -= *it_rhs;
        it_rhs++;
    }
    return *this;
}

template<class T>    // class template
template<class T2>   // function template
TensorBase<T> TensorBase<T>::operator-(const TensorBase<T2>& rhs)
{
    TensorBase<T> result(*this);
    result -= rhs;
    return result;
}

template<class T>
TensorBase<T>& TensorBase<T>::operator*=(const T& rhs)
{
    for(auto it=vector<T>::begin(); it!=vector<T>::end(); it++)
    {
        *it*=rhs;
    }
    return *this;
}

template<class T>
TensorBase<T> TensorBase<T>::operator*(const T& rhs)
{
    TensorBase<T> result(*this);
    result *= rhs;
    return result;
}

template<class T>
TensorBase<T>& TensorBase<T>::operator/=(const T& rhs)
{
    for(auto it=vector<T>::begin(); it!=vector<T>::end(); it++)
    {
        *it/=rhs;
    }
    return *this;
}

template<class T>
TensorBase<T> TensorBase<T>::operator/(const T& rhs)
{
    TensorBase<T> result(*this);
    result /= rhs;
    return result;
}



template<class T>
template<class T2>
TensorBase<T>& TensorBase<T>::operator<<(T2& rhs)
{
    if(is_same<T,T2>::value)
    {
        memcpy( &((*this)[0]), &rhs, sizeof(T)*vector<T>::size() );
    }
    else
    {
        T2* ptr = &rhs;
        for(auto it=vector<T>::begin(); it!=vector<T>::end(); it++)
        {
            *ptr = *it;
            ptr++;
        }
    }
    return *this;
}

template<class T>
template<class T2>
T2& TensorBase<T>::operator>>(T2& rhs)
{
    if(is_same<T,T2>::value)
    {
        memcpy( &rhs, &((*this)[0]), sizeof(T)*vector<T>::size() );
    }
    else
    {
        T2* ptr = &rhs;
        for(auto it=vector<T>::begin(); it!=vector<T>::end(); it++)
        {
            *ptr = *it;
            ptr++;
        }
    }
    return rhs;
}

/**
    OPERATIONS ON SUB-TENSORS
**/

template<class T>
template<class T2>
TensorBase<T>& TensorBase<T>::assign(
    TensorBase<T2>              &rhs,
    const vector<size_t>   &at_lhs,
    const vector<size_t>   &at_rhs)
{
    if(THROW_EXCEPTIONS)
    {
        vector<size_t> subshape_lhs(shape);
        vector<size_t> subshape_rhs(rhs.shape);
        subshape_lhs.erase(subshape_lhs.begin(),subshape_lhs.begin()+at_lhs.size());
        subshape_rhs.erase(subshape_rhs.begin(),subshape_rhs.begin()+at_rhs.size());
        size_t n_elems_lhs = 1;
        size_t n_elems_rhs = 1;
        for(auto it=subshape_lhs.begin(); it!=subshape_lhs.end(); it++)
        {
            n_elems_lhs *= *it;
        }
        for(auto it=subshape_rhs.begin(); it!=subshape_rhs.end(); it++)
        {
            n_elems_rhs *= *it;
        }
        if(n_elems_lhs != n_elems_rhs)
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::assign:: Shape mismatch: Arguments have not the same number of elements!");
        }
    }

    T* lhs_ptr = &(*this)(at_lhs);
    T2* rhs_ptr = &rhs(at_rhs);
    size_t n_max=1;
    for(auto it = shape.begin()+at_lhs.size() ; it!=shape.end(); it++)
    {
        n_max*= *it;
    }
    if(is_same<T,T2>::value)
    {
        memcpy( lhs_ptr , rhs_ptr, sizeof(T)*n_max);
    }
    else
    {
        size_t n=0;
        while(n<n_max)
        {
            *lhs_ptr = *rhs_ptr;
            lhs_ptr++;
            rhs_ptr++;
            n++;
        }
    }
    return *this;
}

template<class T>
template<class T2>
TensorBase<T>& TensorBase<T>::add(
    TensorBase<T2>              &rhs,
    const vector<size_t>   &at_lhs,
    const vector<size_t>   &at_rhs)
{
    if(THROW_EXCEPTIONS)
    {
        vector<size_t> subshape_lhs(shape);
        vector<size_t> subshape_rhs(rhs.shape);
        subshape_lhs.erase(subshape_lhs.begin(),subshape_lhs.begin()+at_lhs.size());
        subshape_rhs.erase(subshape_rhs.begin(),subshape_rhs.begin()+at_rhs.size());
        size_t n_elems_lhs = 1;
        size_t n_elems_rhs = 1;
        for(auto it=subshape_lhs.begin(); it!=subshape_lhs.end(); it++)
        {
            n_elems_lhs *= *it;
        }
        for(auto it=subshape_rhs.begin(); it!=subshape_rhs.end(); it++)
        {
            n_elems_rhs *= *it;
        }
        if(n_elems_lhs != n_elems_rhs)
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::add:: Shape mismatch: Arguments have not the same number of elements!");
        }
    }

    T* lhs_ptr = &(*this)(at_lhs);
    T2* rhs_ptr = &rhs(at_rhs);
    size_t n_max=1;
    for(auto it = shape.begin()+at_lhs.size() ; it!=shape.end(); it++)
    {
        n_max*= *it;
    }
    size_t n=0;
    while(n<n_max)
    {
        *lhs_ptr += *rhs_ptr;
        lhs_ptr++;
        rhs_ptr++;
        n++;
    }
    return *this;
}

template<class T>
template<class T2>
TensorBase<T>& TensorBase<T>::substract(
    TensorBase<T2>              &rhs,
    const vector<size_t>   &at_lhs,
    const vector<size_t>   &at_rhs)
{
    if(THROW_EXCEPTIONS)
    {
        vector<size_t> subshape_lhs(shape);
        vector<size_t> subshape_rhs(rhs.shape);
        subshape_lhs.erase(subshape_lhs.begin(),subshape_lhs.begin()+at_lhs.size());
        subshape_rhs.erase(subshape_rhs.begin(),subshape_rhs.begin()+at_rhs.size());
        size_t n_elems_lhs = 1;
        size_t n_elems_rhs = 1;
        for(auto it=subshape_lhs.begin(); it!=subshape_lhs.end(); it++)
        {
            n_elems_lhs *= *it;
        }
        for(auto it=subshape_rhs.begin(); it!=subshape_rhs.end(); it++)
        {
            n_elems_rhs *= *it;
        }
        if(n_elems_lhs != n_elems_rhs)
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::substract:: Shape mismatch: Arguments have not the same number of elements!");
        }
    }

    T* lhs_ptr = &(*this)(at_lhs);
    T2* rhs_ptr = &rhs(at_rhs);
    size_t n_max=1;
    for(auto it = shape.begin()+at_lhs.size() ; it!=shape.end(); it++)
    {
        n_max*= *it;
    }
    size_t n=0;
    while(n<n_max)
    {
        *lhs_ptr -= *rhs_ptr;
        lhs_ptr++;
        rhs_ptr++;
        n++;
    }
    return *this;
}

template<class T>
TensorBase<T>& TensorBase<T>::multiply( const T &rhs, const vector<size_t> &at_lhs)
{
    T* lhs_ptr = &(*this)(at_lhs);
    size_t n_max=1;
    for(auto it = shape.begin()+at_lhs.size() ; it!=shape.end(); it++)
    {
        n_max*= *it;
    }
    size_t n=0;
    while(n<n_max)
    {
        *lhs_ptr *= rhs;
        lhs_ptr++;
        n++;
    }
    return *this;
}

template<class T>
TensorBase<T>& TensorBase<T>::divide( const T &rhs, const vector<size_t>   &at_lhs)
{
    T* lhs_ptr = &(*this)(at_lhs);
    size_t n_max=1;
    for(auto it = shape.begin()+at_lhs.size() ; it!=shape.end(); it++)
    {
        n_max*= *it;
    }
    size_t n=0;
    while(n<n_max)
    {
        *lhs_ptr /= rhs;
        lhs_ptr++;
        n++;
    }
    return *this;
}

template<class T>
template<class T2>
TensorBase<T> TensorBase<T>::plus(
    TensorBase<T2>              &rhs,
    const vector<size_t>   &at_lhs,
    const vector<size_t>   &at_rhs)
{
    TensorBase<T> result;
    result = this->slice(at_lhs);
    result.add(rhs,{},at_rhs);
    return result;
}

template<class T>
template<class T2>
TensorBase<T> TensorBase<T>::minus(
    TensorBase<T2>              &rhs,
    const vector<size_t>   &at_lhs,
    const vector<size_t>   &at_rhs)
{
    TensorBase<T> result;
    result = this->slice(at_lhs);
    result.substract(rhs,{},at_rhs);
    return result;
}

template<class T>
TensorBase<T> TensorBase<T>::product(const T &rhs, const vector<size_t> &at_lhs)
{
    TensorBase<T> result;
    result = this->slice(at_lhs);
    result *= rhs;
    return result;
}

template<class T>
TensorBase<T> TensorBase<T>::quotient(const T &rhs, const vector<size_t> &at_lhs)
{
    TensorBase<T> result;
    result = this->slice(at_lhs);
    result /= rhs;
    return result;
}

/**
    ACCESS ELEMENTS
**/

template<class T>
T& TensorBase<T>::operator()(const vector<size_t> &indices)
{
    if(indices.empty())
    {
        if(THROW_EXCEPTIONS)
        {
            return vector<T>::at(0);
        }
        return (*this)[0];
    }
    size_t idx = 0;
    if(THROW_EXCEPTIONS)
    {
        if(indices.size()>shape.size())
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        auto it2 = incr.begin();
        auto it3 = shape.begin();
        for(auto it=indices.begin(); it!=indices.end(); it++)
        {
            idx += (*it)*(*it2);
            if((*it)>=(*it3))
            {
                throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
            }
            it2++;
            it3++;
        }
    }
    else
    {
        auto it2 = incr.begin();
        for(auto it=indices.begin(); it!=indices.end(); it++)
        {
            idx += (*it)*(*it2);
            it2++;
        }
    }
    return (*this)[idx];
}

template<class T>
T& TensorBase<T>::operator()(const vector<size_t*> &indices)
{
    if(indices.empty())
    {
        if(THROW_EXCEPTIONS)
        {
            return vector<T>::at(0);
        }
        return (*this)[0];
    }
    size_t idx = 0;
    if(THROW_EXCEPTIONS)
    {
        if(indices.size()>shape.size())
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        auto it2 = incr.begin();
        auto it3 = shape.begin();
        for(auto it=indices.begin(); it!=indices.end(); it++)
        {
            idx += (**it)*(*it2);
            if((**it)>=(*it3))
            {
                throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
            }
            it2++;
            it3++;
        }
    }
    else
    {
        auto it2 = incr.begin();
        for(auto it=indices.begin(); it!=indices.end(); it++)
        {
            idx += (**it)*(*it2);
            it2++;
        }
    }
    return (*this)[idx];
}

template<class T>
T& TensorBase<T>::operator()()
{
    if(THROW_EXCEPTIONS)
    {
        return vector<T>::at(0);
    }
    return (*this)[0];
}

template<class T>
T& TensorBase<T>::operator()(size_t n0)
{
    if(THROW_EXCEPTIONS)
    {
        if(1>shape.size())
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        if(n0>=shape[0])
        {
            throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
        }
    }
    return (*this)[incr[0]*n0];
}

template<class T>
T& TensorBase<T>::operator()(size_t n0, size_t n1)
{
    if(THROW_EXCEPTIONS)
    {
        if(2>shape.size())
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        if(n0>=shape[0]||n1>=shape[1])
        {
            throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
        }
    }
    return (*this)[incr[0]*n0+incr[1]*n1];
}

template<class T>
T& TensorBase<T>::operator()(size_t n0, size_t n1, size_t n2)
{
    if(THROW_EXCEPTIONS)
    {
        if(3>shape.size() )
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        if(n0>=shape[0]||n1>=shape[1]||n2>=shape[2])
        {
            throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
        }
    }
    return (*this)[incr[0]*n0+incr[1]*n1+incr[2]*n2];
}

template<class T>
T& TensorBase<T>::operator()(size_t n0, size_t n1, size_t n2, size_t n3)
{
    if(THROW_EXCEPTIONS)
    {
        if(4>shape.size())
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        if(n0>=shape[0]||n1>=shape[1]||n2>=shape[2]||n3>=shape[3])
        {
            throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
        }
    }
    return (*this)[incr[0]*n0+incr[1]*n1+incr[2]*n2+incr[3]*n3];
}

template<class T>
T& TensorBase<T>::operator()(size_t n0, size_t n1, size_t n2, size_t n3, size_t n4)
{
    if(THROW_EXCEPTIONS)
    {
        if(5>shape.size() )
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        if(n0>=shape[0]||n1>=shape[1]||n2>=shape[2]||n3>=shape[3]||n4>=shape[4])
        {
            throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
        }
    }
    return (*this)[incr[0]*n0+incr[1]*n1+incr[2]*n2+incr[3]*n3+incr[4]*n4];
}

template<class T>
T& TensorBase<T>::operator()(size_t n0, size_t n1, size_t n2, size_t n3, size_t n4, size_t n5)
{
    if(THROW_EXCEPTIONS)
    {
        if(6>shape.size() )
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        if(n0>=shape[0]||n1>=shape[1]||n2>=shape[2]||n3>=shape[3]||n4>=shape[4]||n5>=shape[5])
        {
            throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
        }
    }
    return (*this)[incr[0]*n0+incr[1]*n1+incr[2]*n2+incr[3]*n3+incr[4]*n4+incr[5]*n5];
}

template<class T>
T& TensorBase<T>::operator()(size_t n0, size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6)
{
    if(THROW_EXCEPTIONS)
    {
        if(7>shape.size() )
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        if(n0>=shape[0]||n1>=shape[1]||n2>=shape[2]||n3>=shape[3]||n4>=shape[4]||n5>=shape[5]||n6>=shape[6])
        {
            throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
        }
    }
    return (*this)[incr[0]*n0+incr[1]*n1+incr[2]*n2+incr[3]*n3+incr[4]*n4+incr[5]*n5+incr[6]*n6];
}

template<class T>
T& TensorBase<T>::operator()(size_t n0, size_t n1, size_t n2, size_t n3, size_t n4, size_t n5, size_t n6, size_t n7)
{
    if(THROW_EXCEPTIONS)
    {
        if(8>shape.size() )
        {
            throw ShapeMismatch("TensorUtils::TensorBase<T>::operator():: Too many indices!");
        }
        if(n0>=shape[0]||n1>=shape[1]||n2>=shape[2]||n3>=shape[3]||n4>=shape[4]||n5>=shape[5]||n6>=shape[6]||n7>=shape[7])
        {
            throw out_of_range("TensorUtils::TensorBase<T>::operator():: Index out of range!");
        }
    }
    return (*this)[incr[0]*n0+incr[1]*n1+incr[2]*n2+incr[3]*n3+incr[4]*n4+incr[5]*n5+incr[6]*n6+incr[7]*n7];
}

/**
    EXPLICIT TEMPLATE INSTANTIATION
**/

namespace TensorUtils
{
    #define INSTANTIATE_FUNCTION_TEMPLATES(X,Y) \
    template TensorBase<X>& TensorBase<X>::operator=<Y>(const TensorBase<Y>&); \
    template TensorBase<X>& TensorBase<X>::operator+=<Y>(const TensorBase<Y>&); \
    template TensorBase<X>& TensorBase<X>::operator-=<Y>(const TensorBase<Y>&); \
    template TensorBase<X> TensorBase<X>::operator+<Y>(const TensorBase<Y>&); \
    template TensorBase<X> TensorBase<X>::operator-<Y>(const TensorBase<Y>&); \
    template TensorBase<X>& TensorBase<X>::operator<<<Y>(Y& rhs); \
    template Y& TensorBase<X>::operator>><Y>(Y& rhs); \
    template TensorBase<X>& TensorBase<X>::assign(TensorBase<Y> &rhs, const vector<size_t> &at_lhs, const vector<size_t> &at_rhs); \
    template TensorBase<X>& TensorBase<X>::add(TensorBase<Y> &rhs, const vector<size_t> &at_lhs, const vector<size_t> &at_rhs); \
    template TensorBase<X>& TensorBase<X>::substract(TensorBase<Y> &rhs, const vector<size_t> &at_lhs, const vector<size_t> &at_rhs); \
    template TensorBase<X> TensorBase<X>::plus(TensorBase<Y> &rhs, const vector<size_t> &at_lhs, const vector<size_t> &at_rhs); \
    template TensorBase<X> TensorBase<X>::minus(TensorBase<Y> &rhs, const vector<size_t> &at_lhs, const vector<size_t> &at_rhs); \
    template TensorBase<X> TensorBase<X>::dot(TensorBase<Y>&, const vector<int>&, const vector<int>&, const vector<size_t>&); \
    template void TensorBase<X>::write_bin<Y>(string, string); \
    template void TensorBase<X>::print_helper<Y>(); \
    template void TensorBase<X>::read_txt<Y>(string path); \
    template void TensorBase<X>::read_bin<Y>(string path); \
    template void TensorBase<X>::write_txt<Y>(string, string, int); \

    #define INSTANTIATE_ALL(X) \
    template class TensorBase<X>; \
    INSTANTIATE_FUNCTION_TEMPLATES(X,double) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,float) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,long double) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,unsigned char) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,signed char) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,unsigned short) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,short) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,unsigned) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,int) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,unsigned long) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,long) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,unsigned long long) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,long long) \

    #define INSTANTIATE_FLOATING_POINT_TYPES(X) \
    template class TensorBase<X>; \
    INSTANTIATE_FUNCTION_TEMPLATES(X,double) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,float) \
    INSTANTIATE_FUNCTION_TEMPLATES(X,long double)

    #ifndef ENABLE_INTEGRAL_TYPES
    #define ENABLE_INTEGRAL_TYPES 1
    #endif // ENABLE_INTEGRAL_TYPES

    #if ENABLE_INTEGRAL_TYPES == 1
        INSTANTIATE_ALL(double)
        INSTANTIATE_ALL(float)
        INSTANTIATE_ALL(long double)
        INSTANTIATE_ALL(unsigned char)
        INSTANTIATE_ALL(signed char)
        INSTANTIATE_ALL(unsigned short)
        INSTANTIATE_ALL(short)
        INSTANTIATE_ALL(unsigned)
        INSTANTIATE_ALL(int)
        INSTANTIATE_ALL(unsigned long)
        INSTANTIATE_ALL(long)
        INSTANTIATE_ALL(unsigned long long)
        INSTANTIATE_ALL(long long)
    #else
        INSTANTIATE_FLOATING_POINT_TYPES(double)
        INSTANTIATE_FLOATING_POINT_TYPES(float)
        INSTANTIATE_FLOATING_POINT_TYPES(long double)
    #endif

    #undef INSTANTIATE_ALL
    #undef INSTANTIATE_FLOATING_POINT_TYPES
    #undef INSTANTIATE_FUNCTION_TEMPLATES
}

