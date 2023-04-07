#include <unordered_map>
#include <list>
#include <iostream>

using namespace std;

template <typename K, typename V>
class LruCache
{
public:
    LruCache(size_t max_size) : n(max_size) {}

    void Put(const K &key, const V &value)
    {
        auto p = mp.find(key);
        if (p == mp.end())
        {
            if (mp.size() == n)
            {
                mp.erase(q.back());
                q.pop_back();
            }
            q.push_front(key);
            mp[key]={value, q.begin()};
        }
        else
        {
            q.erase(mp[key].second);
            q.push_front(key);
            mp[key] = {value, q.begin()};
        }
    }

    bool Get(const K &key, V *value)
    {
            auto p = mp.find(key);
            if (p != mp.end())
            {
                q.erase(mp[key].second);
                q.push_front(key);
                mp[key]={(p->second).first,q.begin()};
                *value = mp[key].first;
                return true;
            }
            else
            {
                return false;
            }
        return false;
    }

private:
    list<K> q;
    unordered_map<K, pair<V, typename list<K>::iterator>> mp;
    size_t n;
};
