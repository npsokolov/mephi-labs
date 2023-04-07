#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <string>

using namespace std;

const vector<shared_ptr<string>> DeDuplicate(const vector<unique_ptr<string>> &in)
{

    vector<shared_ptr<string>> out;
    int h = in.size();
    shared_ptr<string> p;
    auto s=in[0].get();
    p = make_shared<string>(*s);
    out.push_back(p);
    vector<string*> v;
    for (int i = 0; i < h; i++)
    {
        v.push_back(in[i].get());
    }
    
    for (int i = 1; i < h; i++)
    {
        int j = i - 1;
        auto s=*v[i];
        while ((j >=0) && (s != *v[j]))
        {
            j--;
        }
        if (j !=-1)
        {
            out.push_back(out[j]);
        }
        else
        {
            out.push_back(make_shared<string>(*v[i]));
        }
    }
    return out;
}

const vector<unique_ptr<string>> Duplicate(const vector<shared_ptr<string>> in)
{
    auto h = in.size();
    vector<unique_ptr<string>> out;
    for (size_t i = 0; i < h; i++)
    {
        out.push_back(std::make_unique<std::string>(*in[i]));
    }
    return out;
}