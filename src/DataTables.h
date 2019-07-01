#include <va/va.h>
#include <unordered_map>
#include <mutex>

using namespace std;

template <typename Key,typename Value>
class DataTable {
private:
    mutex tableMutex;
    unordered_map<Key, Value*> dataTbl;
    Key latestKey = 0;
    
public:
    Key insert(Value* val);
    Value* getValue(Key key);
    /*
     * Removes all data from this table
     */
    void clear();
};

template <typename Key, typename Value>
Key DataTable<Key,Value>::insert(Value* val)
{
    tableMutex.lock();
    Key allocKey = latestKey++;
    dataTbl[allocKey] = val;
    tableMutex.unlock();
    return allocKey;
}

template <typename Key, typename Value>
Value* DataTable<Key, Value>::getValue(Key key)
{
    return dataTbl[key];
}

template<typename Key, typename Value>
void DataTable<Key, Value>::clear()
{
    tableMutex.lock();
    for(auto itr = dataTbl.begin(); itr != dataTbl.end(); itr++) {
        delete itr->second;
    }
    dataTbl.clear();
    tableMutex.unlock();
}
