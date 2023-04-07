#include <cstddef>

using namespace std;

struct State
{
    State() : Size_(0), Capacity_(2), ref_count(1)
    {
        data = new char[2];
    }

    State(size_t size, size_t capacity, char *data_)
    {
        ref_count = 1;
        Size_ = size;
        Capacity_ = capacity;
        data = new char[capacity];
        for (size_t i = 0; i < size; ++i)
        {
            data[i] = data_[i];
        }
    }
    size_t Size_;
    size_t Capacity_;
    int ref_count;
    char *data;


    ~State()
    {
        delete[] data;
    }
};

class CowString
{
public:
    CowString() : str(new State()) {}

    CowString(const CowString &other)
    {
        str=other.str;
        str->ref_count++;
    }

    ~CowString()
    {
        str->ref_count--;
        if (str->ref_count == 0)
        {
            delete str;
        }
    }

    const char &At(size_t index) const
    {
        return *(str->data + index);
    }

    char& operator[](size_t index){
        if(str -> ref_count > 1){
            str->ref_count--;
            str = new State(str->Size_, str->Capacity_, str->data);
        }
        return str->data[index];
    }
    
    CowString operator=(CowString other)
    {
        if (str->ref_count>1)
        {
            str->ref_count--;
        }
        str=other.str;
        str->ref_count++;
        return *this;
    }
    

    const char &Back() const
    {
        return *(str->data + int(Size()) - 1);
    }

    void PushBack(char c)
    {
        if (str->ref_count>1)
        {
            str->ref_count--;
            str=new State(str->Size_, str->Capacity_, str->data);
        }

        if (Size()==Capacity())
        {
            Reserve(2*Size());
        }
        str->data[Size()]=c;
        str->Size_++;
    }

    size_t Size() const
    {
        return str->Size_;
    }
    size_t Capacity() const
    {
        return str->Capacity_;
    }

    void Resize(const size_t newsize)
    {
        if (newsize > Capacity())
        {
            Reserve(newsize);
        }
        str->Size_ = newsize;
    }

    void Reserve(const size_t newcapacity)
    {
        if (newcapacity > Capacity())
        {
            char *data1 = new char[newcapacity];
            for (size_t i = 0; i < Size(); ++i)
            {
                data1[i] = str->data[i];
            }
            delete[] str->data;
            str->data = data1;
            str->Capacity_ = newcapacity;
        }
    }

private:
    State *str;
};
