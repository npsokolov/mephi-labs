#pragma once
#include <cstdint>
#include <cmath>

class Range
{
public:
    class Iterator
    {
    public:
        Iterator(int64_t n, int64_t beg1, int64_t step_) : cur(n), beg(beg1), step(step_) {}

        Iterator operator++(int)
        {
            Iterator copy(*this);
            operator++();
            return copy;
        }

        Iterator operator++()
        {
            cur++;
            return *this;
        }

        int64_t operator*()
        {
            return beg + step * cur;
        }

        bool operator<(const Iterator &it)
        {
            return cur < it.cur;
        }

        bool operator!=(const Iterator &it)
        {
            return it.cur != cur;
        }

        bool operator==(const Iterator &it)
        {
            return it.cur == cur;
        }

    private:
        int64_t cur;
        int64_t beg;
        int64_t step;
    };

    Range(int64_t n) : BeginElem(0), EndElem(n), Shag(1) {}
    Range(int64_t beg, int64_t en) : BeginElem(beg), EndElem(en), Shag(1) {}
    Range(int64_t beg, int64_t en, int64_t step) : BeginElem(beg), EndElem(en), Shag(step) {}

    Iterator begin()
    {
        return Iterator(0,BeginElem,Shag);
    }

    Iterator begin() const
    {
        return Iterator(0,BeginElem,Shag);
    }


    Iterator end() const
    {
        return Iterator(Size(),BeginElem,Shag);
    }

    Iterator end()
    {
        return Iterator(Size(),BeginElem,Shag);
    }

    int64_t Size() const
    {
        return ceil((EndElem - BeginElem) / double(Shag));
    }

private:
    int64_t BeginElem;
    int64_t EndElem;
    int64_t Shag;
};
