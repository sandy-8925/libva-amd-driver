#include "DataTables.h"

template <class Key, class Value>
Key DataTable<Key,Value>::insert(Value* val)
{
    tableMutex.lock();
    Key allocKey = latestKey++;
    dataTbl[allocKey] = val;
    tableMutex.unlock();
    return allocKey;
}

template <class Key, class Value>
Value* DataTable<Key,Value>::getValue(Key key)
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
