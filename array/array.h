#pragma once
#include <iostream>
#include <iterator>
template <typename T>
class ArrayIterator : public std::iterator<std::random_access_iterator_tag, T>
{

public:
    using difference_type = std::ptrdiff_t;
    ArrayIterator(T *ptr) : cur_ptr(ptr) {}

    ArrayIterator &operator++()
    { 
        ++cur_ptr;
        return *this;
    }
    
    ArrayIterator operator++(int)
    { 
        return ArrayIterator(cur_ptr++);
    }
    ArrayIterator &operator--()
    {
        --cur_ptr;
        return *this;
    }
    ArrayIterator operator--(int)
    {
        return ArrayIterator(cur_ptr--);
    }

    ArrayIterator operator+(const difference_type val) const
    { 
        return ArrayIterator(cur_ptr + val);
    }

    ArrayIterator operator+(const ArrayIterator &other) const
    {
        return ArrayIterator(*this + other.cur_ptr);
    }

    bool operator<=(const ArrayIterator &a) const
    {
        return !(operator>(a));
    }

    bool operator>=(const ArrayIterator &a) const
    {
        return !(operator<(a));
    }

    bool operator==(const ArrayIterator &a) const
    {
        return a.cur_ptr == cur_ptr;
    }
    bool operator!=(const ArrayIterator &a) const
    {
        return a.cur_ptr != cur_ptr;
    }

    ArrayIterator operator-(const difference_type val) const
    {
        return ArrayIterator(cur_ptr - val);
    }

    difference_type operator-(const ArrayIterator &other) const
    {
        return std::distance(other.cur_ptr, cur_ptr);
    }

    ArrayIterator &operator+=(const difference_type other)
    {
        cur_ptr += other;
        return *this;
    }

    T &operator*()
    {
        return *cur_ptr;
    }

    T &operator[](size_t val) const
    {
        return cur_ptr[val];
    }
    
    ArrayIterator &operator-=(const difference_type other)
    {
        cur_ptr -= other;
        return *this;
    }

    bool operator<(const ArrayIterator &a) const
    {
        return cur_ptr < a.cur_ptr;
    }

    bool operator>(const ArrayIterator &a) const
    {
        return !(operator<(a));
    }

    T &operator*() const
    {
        return *cur_ptr;
    }

    T *operator->()
    {

        return *(&this->cur_ptr);
    }

private:
    T *cur_ptr;
};

template <typename T>
ArrayIterator<T> operator+(const int val, const ArrayIterator<T> &it)
{
    return it + val;
}

template <typename T>
class Array
{
    friend std::ostream& operator<<(std::ostream& ostream, const Array<T>& array){
        if (array.Size() == 0){
            ostream << "Result Array's capacity is " << array.Capacity() << " and size is " << 0;
        }else{
            ostream << "Result Array's capacity is " << array.Capacity() << " and size is " << array.Size()
                << ", elements are: ";
            for (size_t i = 0; i < array.Size() - 1; ++i){
                ostream << *(array.Elements_ + i) << ", ";
            }
            ostream << *(array.Elements_ + array.Size() - 1);
        }
        return ostream;
    }
    T *Elements_;
    size_t Size_;
    size_t Capacity_;
    std::ostream &Ostream_;
    friend class iterator;
public:
    Array(std::ostream &ostream) : Ostream_(ostream)
    {
        Size_ = 0;
        Capacity_ = 2;
        Elements_ = new T[2];
        Ostream_ << "Constructed. " << *this << "\n";
    }
    Array(const Array<T> &array) : Ostream_(array.Ostream_)
    {
        Capacity_ = array.Capacity();
        Size_ = array.Size();
        Elements_ = new T[Capacity()];
        for (size_t i = 0; i < Size_; i++)
        {
            Elements_[i] = array[i];
        }
        Ostream_ << "Constructed from another Array. " << *this << "\n";//!
    
    }
    Array(size_t size, std::ostream &ostream = std::cout, T defaultValue = T()) : Ostream_(ostream)
    {
        Size_ = size;
        Capacity_ = Size_ * 2;
        Elements_ = new T[Capacity()];
        for (size_t i = 0; i < Size_; i++)
        {
            Elements_[i] = defaultValue;
        }
        Ostream_ << "Constructed with default. "<< *this << "\n";//!

    }

    ~Array()
    {
        delete[] Elements_;
        Ostream_ << "Destructed " << Size_ << "\n";
    }

    size_t Size() const
    {
        return Size_;
    }
    size_t Capacity() const
    {
        return Capacity_;
    }
    ArrayIterator<T> begin() const
    {
        return ArrayIterator(Elements_);
    }
    ArrayIterator<T> end() const
    {
        return ArrayIterator(Elements_ + Size_);
    }
    ArrayIterator<T> begin()
    {
        return ArrayIterator(Elements_);
    }
    ArrayIterator<T> end()
    {
        return ArrayIterator(Elements_ + Size_);
    }

    void Reserve(size_t newCapacity)
    {
        if (newCapacity > Capacity_)
        {
            Capacity_ = newCapacity;
            T *arr = new T[newCapacity];
            for (size_t i = 0; i < Size_; i++)
            {
                arr[i] = Elements_[i];
            }

            delete[] Elements_;
            Elements_ = arr;
            Capacity_ = newCapacity;
        }
    }

    void Resize(size_t newSize)
    {
        if (newSize > Capacity())
        {
            Reserve(newSize);
        }
        Size_ = newSize;
    }

    void PushBack(T value = 0)
    {
        if (Size_ == Capacity_)
        {
            Reserve(Size_ * 2);
        }
        Elements_[Size_] = value;
        Size_++;
    }

    void PopBack()
    {
        Size_--;
    }

    const T &operator[](const size_t i) const
    {
        return Elements_[i];    
    }

    T &operator[](const size_t i)
    {
        return Elements_[i];
    }

    explicit operator bool() const
    {
        return Elements_ != NULL;
    }

    bool operator<(const Array<T> &it) const
    {
        size_t m;
        if (Size_ < it.Size())
        {
            m = Size_;
        }
        else
        {
            m = it.Size();
        }
        for (size_t i = 0; i < m; i++)
        {
            if (Elements_[i] < it[i])
            {
                return true;
            }
            else if (Elements_[i] > it[i])
            {
                return false;
            }
        }
        if (Size_ < it.Size())
        {
            return true;
        }
        return false;
    }

    bool operator>(const Array<T> &it) const
    {
        return !(*this < it || *this == it);
    }

    bool operator!=(const Array<T> &it) const
    {
        return !(*this == it);
    }

    bool operator==(const Array<T> &it) const
    {
        if (Size_ == it.Size())
        {
            for (size_t i = 0; i < Size_; i++)
            {
                if (Elements_[i] != it[i])
                {
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    bool operator<=(const Array<T> &it) const
    {
        return (*this < it || *this == it);
    }

    bool operator>=(const Array<T> &it) const
    {
        return !(*this < it);
    }

    Array<T> &operator<<(const T &value)
    {
        
        T *arr = new T[Size_ + 1];
        if (Size_ == Capacity_)
        {
            Capacity_ *= 2;
        }
        for (size_t i = 0; i < Size_; i++)
        {
            arr[i] = Elements_[i];
        }
        arr[Size_] = value;
        Size_++;
        delete[] Elements_;
        Elements_ = arr;
        return *this;
    }

    Array<T> &operator<<(const Array<T> &it)
    {
        for (size_t i = 0; i < it.Size(); i++)
        {
            *this << it[i];
        }
        return *this;
    }

    bool Insert(size_t pos, const T &value)
    {
        if (pos > Size_)
        {
            return false;
        }
        else
        {
            
           if(Size() == Capacity()){
                Reserve(Capacity() * 2);
            }
            if(pos == Size()){
                PushBack(value);
            }else{
                if(pos != 0){
                    pos--;
                }
                T mem1 = Elements_[pos];
                T mem2;
                Elements_[pos] = value;
                for(size_t i = pos + 1; i < Size(); i++){
                    mem2 = Elements_[i];
                    Elements_[i] = mem1;
                    mem1 = mem2;
                }
                Size_++;
            }
        }
        return true;
    }

    bool Erase(size_t pos)
    {
        if (pos >= Size_)
        {
            return false;
        }
        else
        {
            T *arr = new T[Size_ - 1];
            for (size_t i = 0; i < pos; i++)
            {
                arr[i] = Elements_[i];
            }
            for (size_t i = pos; i < Size_ - 1; i++)
            {
                arr[i] = Elements_[i + 1];
            }
            Size_--;
            delete[] Elements_;
            Elements_ = arr;
        }
        return true;
    }
};
