#pragma once

#include <iostream>
#include <assert.h>
#include <math.h>
#include <fstream>
#include "../exception/myexception.h"
#include "errors.h"

const double EPS = 1e-6;
enum interpolation
{
    NONE = 0,
    LINEAR = 1,
    BEZIER2 = 2,
    BEZIER3 = 3,
    CATROM = 4,
    LAGRAN = 5
};


template <typename DataT>
class Vector
{
    private:
        static const int capacity;
        static const int POISON;

        DataT* buffer_          = nullptr;
        int size_               = POISON;
        int capacity_           = POISON;

        int interpolation_      = NONE;

    public:
        Vector                        ();
        Vector                        (size_t capacity__);
        Vector                        (Vector&& victim);
        Vector                        (const Vector& that);
        ~Vector                       ();

        int Size                      ();

        const DataT& operator []      (const int pos) const;
        DataT& operator []            (const int pos);
        void* operator new            (size_t size_, void* place);
        void swap                     (Vector& that);
        Vector& operator =            (const Vector& that);
        Vector& operator =            (Vector&& that);

        double operator []            (const double pos);
        void set_interpolation        (const int mode);
};

template <typename DataT>
const int Vector <DataT>::capacity = 256;

template <typename DataT>
const int Vector <DataT>::POISON   = -100;


template <typename DataT>
Vector <DataT>::Vector ():
    buffer_(new DataT[capacity]{}),
    size_(0),
    capacity_(capacity),
    interpolation_(0)

{
    if (size_ == POISON || capacity_ == POISON || buffer_ == nullptr)
        MAKE_EXCEPTION("Bad constructing", BAD_CONSTR, nullptr);
}

template <typename DataT>
Vector <DataT>::~Vector ()
{
    delete [] buffer_;
    buffer_         = nullptr;

    size_           = POISON;
    capacity_       = POISON;

    interpolation_  = 0;
}

template <typename DataT>
int Vector <DataT>::Size                ()
{
    return capacity_;
}

template <typename DataT>
const DataT& Vector <DataT>::operator [] (const int pos) const
{
    //assert (pos > 0);
    //assert (pos < capacity_ - 1);
    if (pos >= 0 && pos < capacity_)
        return buffer_[pos];

    else
        MAKE_EXCEPTION("Bad index", BAD_IND, nullptr);
}

template <typename DataT>
DataT& Vector <DataT>::operator [] (const int pos)
{
    if (pos >= 0 && pos < capacity_)
        return (buffer_[pos]);

    else
        MAKE_EXCEPTION("Bad index", BAD_IND, nullptr);
}

template <typename DataT>
void* Vector <DataT>::operator new (size_t size_, void* place)
{
    return place;
}

/*template <typename DataT>
Vector <DataT>& Vector <DataT>::operator = (const Vector& that)
{
    this -> ~Vector ();

    new (this) Vector (that);
}*/

template <typename DataT>
void Vector <DataT>::swap (Vector& that)
{
    std::swap (size_, that.size_);
    std::swap (capacity_, that.capacity_);
    std::swap (buffer_, that.buffer_);
    std::swap (interpolation_, that.interpolation_);
}

template <typename DataT>
Vector <DataT>& Vector <DataT>::operator = (const Vector& that)
{
    Vector temp (that);
    temp.swap   (*this);
}

template <typename DataT>
Vector <DataT>& Vector <DataT>::operator = (Vector&& that)
{
    swap (that);
}

template <typename DataT>
Vector <DataT>::Vector (size_t capacity__):
    buffer_                (new DataT [capacity__]{}),
    size_                  (0),
    capacity_              (capacity__),
    interpolation_         (0)

{
    if (size_ == POISON || capacity_ == POISON || buffer_ == nullptr)
        MAKE_EXCEPTION("Bad constructing", BAD_CONSTR, nullptr);
}


template <typename DataT>
Vector <DataT>::Vector (Vector&& victim):
    buffer_                 (victim.buffer_),
    size_                   (victim.size_),
    capacity_               (victim.capacity_),
    interpolation_          (0)

{
    victim.buffer_ = nullptr;
    if (size_ == POISON || capacity_ == POISON || buffer_ == nullptr)
        MAKE_EXCEPTION("Bad constructing", BAD_CONSTR, nullptr);
}

template <typename DataT>
Vector <DataT>::Vector (const Vector& that)
{
    ~Vector();
    Vector victim (that.capacity_);
    std::copy (that.buffer_, that.buffer + that.capacity_, victim.buffer_);
    swap (victim);
}

template <>
void Vector <double>::set_interpolation (const int mode)
{
    interpolation_ = mode;
}

template <>
double Vector <double>::operator [] (const double pos)
{
    if (pos < 0 || pos > capacity_ - 1)
        MAKE_EXCEPTION("Bad index", BAD_IND, nullptr);

    if (abs (pos - (int) (pos)) < EPS)
        return operator [] ((int) pos);

    double t = 0;
    int ind = 0;
    double result = 0;

    switch (interpolation_)
    {
        case LINEAR:
            if (capacity_ < 2)
                MAKE_EXCEPTION("Not enough points to make linear interpolation", ER_LIN, nullptr);

            ind = (int) pos;
            t = pos - (int) pos;

            return (1 - t) * buffer_[ind] + t * buffer_[ind + 1];
            break;

        case BEZIER2:
            if (capacity_ < 3)
                MAKE_EXCEPTION("Not enough points to make Bezier interpolation", ER_BEZ1, nullptr);

            ind = ((int) pos / 2) * 2;
            t = (pos - ind) / 2;

            if (ind >= capacity_ - 2)
            {
                ind -= 1;
                t += 0.5;
            }

            return (buffer_[ind] * (1 - t) + buffer_[ind + 1] * t) * (1 - t) + (buffer_[ind + 1] * (1 - t) + buffer_[ind + 2] * t) * t;


        case BEZIER3:
            if (capacity_ < 4)
                MAKE_EXCEPTION("Not enough points to make Bezier interpolation", ER_BEZ2, nullptr);

            ind = ((int) pos / 3) * 3;
            t = (pos - ind) / 3;

            if (ind == capacity_ - 3)
            {
                ind -= 1;
                t += 1 / 3;
            }

            else if (ind == capacity_ - 2)
            {
                ind -= 2;
                t += 2 / 3;
            }

            return ((buffer_[ind] * (1 - t) + buffer_[ind + 1] * t) * (1 - t) + (buffer_[ind + 1] * (1 - t) + buffer_[ind + 2] * t) * t) * (1 - t) +
                       ((buffer_[ind + 1] * (1 - t) + buffer_[ind + 2] * t) * (1 - t) + (buffer_[ind + 2] * (1 - t) + buffer_[ind + 3] * t) * t) * t;

        case CATROM:
            if (capacity_ < 4)
                MAKE_EXCEPTION("Not enough points to make Catmull Rom interpolation", ER_CATR, nullptr);

            ind = (int) pos;
            t = pos - ind;

            if (ind == capacity_ - 3)
                ind--;
            else if (ind == capacity_ - 2)
                ind -= 2;

            return 0.5 * ((- t * t * t + 2 * t * t - t) * buffer_[ind] + (3 * t * t * t - 5 * t * t + 2) * buffer_[ind + 1]
                        + (- 3 * t * t * t + 4 * t * t + t) * buffer_[ind + 2] + (t * t * t - t * t) * buffer_[ind + 3]);

            break;

        case LAGRAN:
            if (capacity_ < 2)
                MAKE_EXCEPTION("Not enough points to make Lagrange interpolation", ER_LAGRAN, nullptr);

            result = 0;

            for (int i = 0; i < capacity_; i++)
            {
                double factor = 1;

                for (int j = 0; j < capacity_; j++)
                {
                    if (i != j)
                        factor *= (pos - j) / (i - j);
                }

                result += factor * buffer_[i];
            }

            return result;

        default:
            MAKE_EXCEPTION("The interpolation mode is not chosen", ER_MODE, nullptr);
    }
}

template <>
class Vector <bool>
{
    private:

        class Proxy
        {
            private:
                int _shift = 0;
                char* _ptr = nullptr;

            public:
                Proxy () = delete;
                ~Proxy () = default;

                Proxy( int shift, char* ptr ):
                    _shift( shift ),
                    _ptr( ptr )
                {};

                operator bool() const
                {
                    return (bool)( (*_ptr) & (1 << _shift) );
                }

                Proxy& operator = (bool value)
                {
                   if( value )
                   {
                       (*_ptr) |= (1 << _shift);
                   }

                   else
                   {
                       (*_ptr) &= ~(1 << _shift);
                   }

                   return *this;
                }
        };

        char* _data      = nullptr;

        size_t _size       = 0;
        size_t _capacity   = 0;


    public:

        Vector ()  = default;
        Vector (const Vector& that);
        Vector (Vector&& that);
        Vector (size_t capacity);

        Vector& operator= (const Vector& that);
        Vector& operator= (Vector&& that);

        ~Vector ();

        Proxy operator [] (int index) const;

};

Vector<bool>::Vector (const Vector<bool>& that):
    _size (that._size),
    _capacity (that._capacity)

{
    _data = new (std::nothrow) char[_capacity / 8  + 1]{};

    std::copy (_data, _data + _size / 8 + 1, that._data);
}

Vector<bool>::Vector (Vector<bool> &&that)
{
   std::swap (_size, that._size);
   std::swap (_capacity, that._capacity);
   std::swap (_data, that._data);
}

Vector<bool>::Vector (size_t size):
    _size (size ),
    _capacity (size)

{
    _data = new (std::nothrow) char[_size / 8  + 1]{};
}

Vector<bool>& Vector<bool>::operator = (const Vector& that)
{
    char* new_data = new (std::nothrow) char[that._capacity / 8  + 1]{};

    std::copy (new_data, new_data + that._capacity / 8 + 1, that._data);

    delete[] _data;

    _data       = new_data;
    _size       = that._size;
    _capacity   = that._capacity;

    return *this;
}

Vector<bool>& Vector<bool>::operator = (Vector&& that)
{
   new (this) Vector (std::move (that));
   return *this;
}

Vector<bool>::~Vector ()
{
    delete[] _data;
}

Vector <bool>::Proxy Vector <bool>::operator [] (int index) const
{
   Proxy   tmp (index % 8, (char*)(_data + index / 8));

   return  tmp;
}
